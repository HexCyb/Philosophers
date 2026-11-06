#include "philo.h"

int	is_stopped(t_table *t)
{
	int	stop;

	pthread_mutex_lock(&t->stop_lock);
	stop = t->stop;
	pthread_mutex_unlock(&t->stop_lock);
	return (stop);
}

void	set_stop(t_table *t)
{
	pthread_mutex_lock(&t->stop_lock);
	t->stop = 1;
	pthread_mutex_unlock(&t->stop_lock);
}

/*
** The timestamp is taken while holding print_lock, so the lines come out in
** time order, and nothing is printed once the simulation has stopped.
*/
void	say(t_philo *p, char *message)
{
	pthread_mutex_lock(&p->table->print_lock);
	if (!is_stopped(p->table))
		printf("%ld %d %s\n", now_ms() - p->table->start, p->id, message);
	pthread_mutex_unlock(&p->table->print_lock);
}
