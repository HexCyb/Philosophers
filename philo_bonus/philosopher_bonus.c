#include "philo_bonus.h"

/* Runs beside the philosopher in the same process and notices its death. */
static void	*watch_self(void *arg)
{
	t_table	*t;
	long	since;
	int		done;

	t = (t_table *)arg;
	while (1)
	{
		sem_wait(t->meal);
		since = now_ms() - t->last_meal;
		done = t->done;
		sem_post(t->meal);
		if (done)
			return (NULL);
		if (since > t->time_to_die)
		{
			sem_wait(t->print);
			put_status(now_ms() - t->start, t->id, "died");
			exit(1);
		}
		usleep(500);
	}
	return (NULL);
}

static void	eat(t_table *t)
{
	sem_wait(t->seats);
	sem_wait(t->forks);
	say(t, "has taken a fork");
	sem_wait(t->forks);
	say(t, "has taken a fork");
	sem_wait(t->meal);
	t->last_meal = now_ms();
	sem_post(t->meal);
	say(t, "is eating");
	nap(t->time_to_eat);
	t->meals++;
	sem_post(t->forks);
	sem_post(t->forks);
	sem_post(t->seats);
}

/*
** Full: the watcher is told to stop and joined before the process exits, so
** it cannot report a death while exit() runs.
*/
static void	leave_full(t_table *t)
{
	sem_wait(t->meal);
	t->done = 1;
	sem_post(t->meal);
	pthread_join(t->watcher, NULL);
	sem_close(t->meal);
	close_sems(t);
	exit(0);
}

static void	start_watcher(t_table *t)
{
	char	name[32];

	sem_name(name, "meal", t->id);
	t->meal = open_private_sem(name, 1);
	if (t->meal == SEM_FAILED)
		exit(2);
	if (pthread_create(&t->watcher, NULL, watch_self, t))
		exit(2);
}

/* The life of one child process. It never returns. */
void	philosopher(t_table *t, int id)
{
	t->id = id;
	t->meals = 0;
	t->last_meal = t->start;
	free(t->pids);
	t->pids = NULL;
	start_watcher(t);
	if (id % 2 == 0)
	{
		say(t, "is thinking");
		nap(t->time_to_eat / 2);
	}
	while (1)
	{
		eat(t);
		if (t->must_eat > 0 && t->meals >= t->must_eat)
			leave_full(t);
		say(t, "is sleeping");
		nap(t->time_to_sleep);
		say(t, "is thinking");
		nap(t->think_time);
	}
}
