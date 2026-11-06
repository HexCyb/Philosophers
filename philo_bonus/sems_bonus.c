#include "philo_bonus.h"

/*
** Created, then unlinked at once: the name disappears from the system but
** the semaphore stays usable here and in every child forked afterwards, so
** nothing is left behind even if the program is killed.
*/
sem_t	*open_private_sem(char *name, int value)
{
	sem_t	*sem;

	sem_unlink(name);
	sem = sem_open(name, O_CREAT | O_EXCL, 0600, value);
	sem_unlink(name);
	return (sem);
}

static int	seats(int count)
{
	if (count / 2 < 1)
		return (1);
	return (count / 2);
}

int	open_sems(t_table *t)
{
	char	name[32];

	sem_name(name, "forks", 0);
	t->forks = open_private_sem(name, t->count);
	sem_name(name, "seats", 0);
	t->seats = open_private_sem(name, seats(t->count));
	sem_name(name, "print", 0);
	t->print = open_private_sem(name, 1);
	if (t->forks == SEM_FAILED || t->seats == SEM_FAILED
		|| t->print == SEM_FAILED)
	{
		close_sems(t);
		return (1);
	}
	return (0);
}

void	close_sems(t_table *t)
{
	if (t->forks && t->forks != SEM_FAILED)
		sem_close(t->forks);
	if (t->seats && t->seats != SEM_FAILED)
		sem_close(t->seats);
	if (t->print && t->print != SEM_FAILED)
		sem_close(t->print);
	t->forks = NULL;
	t->seats = NULL;
	t->print = NULL;
}
