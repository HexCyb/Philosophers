#ifndef PHILO_H
# define PHILO_H

# include <pthread.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

# define MAX_PHILOS 200

typedef struct s_table	t_table;

/*
** One philosopher = one thread. "first" and "second" are the two forks it
** touches, already ordered so that neighbours never wait on each other in a
** circle (odd ids take the left fork first, even ids the right one).
** last_meal and meals are read by the monitor, so meal_lock guards them.
*/
typedef struct s_philo
{
	int				id;
	int				meals;
	long			last_meal;
	pthread_mutex_t	meal_lock;
	pthread_mutex_t	*first;
	pthread_mutex_t	*second;
	pthread_t		thread;
	t_table			*table;
}	t_philo;

struct s_table
{
	int				count;
	long			time_to_die;
	long			time_to_eat;
	long			time_to_sleep;
	long			think_time;
	int				must_eat;
	long			start;
	int				stop;
	int				started;
	pthread_mutex_t	stop_lock;
	pthread_mutex_t	print_lock;
	pthread_mutex_t	*forks;
	t_philo			*philos;
};

/* parse.c */
int		parse_args(int argc, char **argv, t_table *table);

/* init.c */
int		init_table(t_table *table);
void	destroy_table(t_table *table);

/* time.c */
long	now_ms(void);
void	nap(t_table *table, long ms);

/* state.c */
int		is_stopped(t_table *table);
void	set_stop(t_table *table);
void	say(t_philo *philo, char *message);

/* routine.c */
void	*philo_routine(void *arg);

/* monitor.c */
void	watch(t_table *table);

#endif
