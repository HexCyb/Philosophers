#include "philo_bonus.h"

static void	stop_all(t_table *t)
{
	int	i;

	i = 0;
	while (i < t->count)
	{
		if (t->pids[i] > 0)
			kill(t->pids[i], SIGKILL);
		i++;
	}
	i = 0;
	while (i < t->count)
	{
		if (t->pids[i] > 0)
			waitpid(t->pids[i], NULL, 0);
		i++;
	}
}

static void	forget_pid(t_table *t, pid_t pid)
{
	int	i;

	i = 0;
	while (i < t->count)
	{
		if (t->pids[i] == pid)
			t->pids[i] = 0;
		i++;
	}
}

/*
** A child that exits with 0 has eaten enough. Anything else (a death, an
** error, a signal) ends the whole simulation.
*/
static void	wait_all(t_table *t)
{
	int		done;
	int		status;
	pid_t	pid;

	done = 0;
	while (done < t->count)
	{
		pid = waitpid(-1, &status, 0);
		if (pid < 0)
			break ;
		forget_pid(t, pid);
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			break ;
		done++;
	}
	stop_all(t);
}

static int	spawn_all(t_table *t)
{
	int		i;
	pid_t	pid;

	t->start = now_ms();
	i = 0;
	while (i < t->count)
	{
		pid = fork();
		if (pid < 0)
		{
			stop_all(t);
			return (error("cannot fork"));
		}
		if (pid == 0)
			philosopher(t, i + 1);
		t->pids[i++] = pid;
	}
	return (0);
}

int	main(int argc, char **argv)
{
	t_table	table;

	if (parse_args(argc, argv, &table))
		return (error("usage: ./philo_bonus number_of_philosophers "
				"time_to_die time_to_eat time_to_sleep "
				"[number_of_times_each_philosopher_must_eat]\n"
				"       (positive integers, at most 200 philosophers)"));
	table.pids = malloc(sizeof(pid_t) * table.count);
	if (!table.pids)
		return (error("out of memory"));
	memset(table.pids, 0, sizeof(pid_t) * table.count);
	if (open_sems(&table))
	{
		free(table.pids);
		return (error("cannot create the semaphores"));
	}
	if (spawn_all(&table) == 0)
		wait_all(&table);
	close_sems(&table);
	free(table.pids);
	return (0);
}
