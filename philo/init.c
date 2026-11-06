#include "philo.h"

static int	init_forks(t_table *t)
{
	int	i;

	t->forks = malloc(sizeof(pthread_mutex_t) * t->count);
	if (!t->forks)
		return (1);
	i = 0;
	while (i < t->count)
	{
		if (pthread_mutex_init(&t->forks[i], NULL))
		{
			while (--i >= 0)
				pthread_mutex_destroy(&t->forks[i]);
			free(t->forks);
			t->forks = NULL;
			return (1);
		}
		i++;
	}
	return (0);
}

static void	seat_philo(t_table *t, t_philo *p, int i)
{
	pthread_mutex_t	*left;
	pthread_mutex_t	*right;

	p->id = i + 1;
	p->table = t;
	left = &t->forks[i];
	right = &t->forks[(i + 1) % t->count];
	p->first = left;
	p->second = right;
	if (p->id % 2 == 0)
	{
		p->first = right;
		p->second = left;
	}
}

static int	init_philos(t_table *t)
{
	int	i;

	t->philos = malloc(sizeof(t_philo) * t->count);
	if (!t->philos)
		return (1);
	memset(t->philos, 0, sizeof(t_philo) * t->count);
	i = 0;
	while (i < t->count)
	{
		if (pthread_mutex_init(&t->philos[i].meal_lock, NULL))
		{
			while (--i >= 0)
				pthread_mutex_destroy(&t->philos[i].meal_lock);
			free(t->philos);
			t->philos = NULL;
			return (1);
		}
		seat_philo(t, &t->philos[i], i);
		i++;
	}
	return (0);
}

int	init_table(t_table *t)
{
	if (pthread_mutex_init(&t->stop_lock, NULL))
		return (1);
	if (pthread_mutex_init(&t->print_lock, NULL))
	{
		pthread_mutex_destroy(&t->stop_lock);
		return (1);
	}
	if (init_forks(t) || init_philos(t))
	{
		destroy_table(t);
		return (1);
	}
	return (0);
}

void	destroy_table(t_table *t)
{
	int	i;

	i = 0;
	while (t->philos && i < t->count)
		pthread_mutex_destroy(&t->philos[i++].meal_lock);
	i = 0;
	while (t->forks && i < t->count)
		pthread_mutex_destroy(&t->forks[i++]);
	free(t->philos);
	free(t->forks);
	t->philos = NULL;
	t->forks = NULL;
	pthread_mutex_destroy(&t->print_lock);
	pthread_mutex_destroy(&t->stop_lock);
}
