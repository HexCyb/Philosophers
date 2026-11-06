#ifndef PHILO_BONUS_H
# define PHILO_BONUS_H

# include <fcntl.h>
# include <pthread.h>
# include <semaphore.h>
# include <signal.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <sys/wait.h>
# include <unistd.h>

# define MAX_PHILOS 200

/*
** Every philosopher is a process. The forks lie in the middle of the table:
** "forks" counts them. At most count / 2 philosophers may reach for forks at
** the same time ("seats"), so they can never all hold one fork and wait.
** "print" keeps lines whole; the process that dies keeps it forever, so no
** line can follow "died". "meal" is private to each process and protects
** last_meal and done between the philosopher and its own watcher thread.
*/
typedef struct s_table
{
	int			count;
	long		time_to_die;
	long		time_to_eat;
	long		time_to_sleep;
	long		think_time;
	int			must_eat;
	long		start;
	int			id;
	int			meals;
	long		last_meal;
	int			done;
	sem_t		*forks;
	sem_t		*seats;
	sem_t		*print;
	sem_t		*meal;
	pthread_t	watcher;
	pid_t		*pids;
}	t_table;

/* parse_bonus.c */
int		parse_args(int argc, char **argv, t_table *table);

/* time_bonus.c */
long	now_ms(void);
void	nap(long ms);

/* output_bonus.c */
void	say(t_table *table, char *message);
void	put_status(long ms, int id, char *message);
int		error(char *message);
void	sem_name(char *buf, char *kind, int id);

/* sems_bonus.c */
int		open_sems(t_table *table);
void	close_sems(t_table *table);
sem_t	*open_private_sem(char *name, int value);

/* philosopher_bonus.c */
void	philosopher(t_table *table, int id);

#endif
