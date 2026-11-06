#include "philo.h"

static int	starved(t_philo *p)
{
	long	since;

	pthread_mutex_lock(&p->meal_lock);
	since = now_ms() - p->last_meal;
	pthread_mutex_unlock(&p->meal_lock);
	return (since > p->table->time_to_die);
}

static int	full(t_philo *p)
{
	int	meals;

	if (p->table->must_eat < 0)
		return (0);
	pthread_mutex_lock(&p->meal_lock);
	meals = p->meals;
	pthread_mutex_unlock(&p->meal_lock);
	return (meals >= p->table->must_eat);
}

/* Stop first, then print: no other line can appear after "died". */
static void	announce_death(t_table *t, t_philo *p)
{
	pthread_mutex_lock(&t->print_lock);
	set_stop(t);
	printf("%ld %d died\n", now_ms() - t->start, p->id);
	pthread_mutex_unlock(&t->print_lock);
}

/* One pass over the table. Returns 1 when the simulation has to stop. */
static int	check_round(t_table *t)
{
	int	i;
	int	satisfied;

	i = 0;
	satisfied = 0;
	while (i < t->count)
	{
		if (starved(&t->philos[i]))
		{
			announce_death(t, &t->philos[i]);
			return (1);
		}
		satisfied += full(&t->philos[i]);
		i++;
	}
	if (satisfied == t->count)
	{
		set_stop(t);
		return (1);
	}
	return (0);
}

/* The main thread looks at every philosopher about every half millisecond. */
void	watch(t_table *t)
{
	while (!check_round(t))
		usleep(500);
}
