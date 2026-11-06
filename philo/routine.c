#include "philo.h"

static void	eat(t_philo *p)
{
	pthread_mutex_lock(p->first);
	say(p, "has taken a fork");
	pthread_mutex_lock(p->second);
	say(p, "has taken a fork");
	pthread_mutex_lock(&p->meal_lock);
	p->last_meal = now_ms();
	pthread_mutex_unlock(&p->meal_lock);
	say(p, "is eating");
	nap(p->table, p->table->time_to_eat);
	pthread_mutex_lock(&p->meal_lock);
	p->meals++;
	pthread_mutex_unlock(&p->meal_lock);
	pthread_mutex_unlock(p->second);
	pthread_mutex_unlock(p->first);
}

/* A single philosopher owns one fork and can only wait for the end. */
static void	*alone(t_philo *p)
{
	pthread_mutex_lock(p->first);
	say(p, "has taken a fork");
	while (!is_stopped(p->table))
		usleep(500);
	pthread_mutex_unlock(p->first);
	return (NULL);
}

/*
** Even philosophers start by thinking for half a meal: the odd ones take
** their forks first and nobody grabs a single fork and blocks a neighbour.
*/
void	*philo_routine(void *arg)
{
	t_philo	*p;

	p = (t_philo *)arg;
	if (p->table->count == 1)
		return (alone(p));
	if (p->id % 2 == 0)
	{
		say(p, "is thinking");
		nap(p->table, p->table->time_to_eat / 2);
	}
	while (!is_stopped(p->table))
	{
		eat(p);
		say(p, "is sleeping");
		nap(p->table, p->table->time_to_sleep);
		say(p, "is thinking");
		nap(p->table, p->table->think_time);
	}
	return (NULL);
}
