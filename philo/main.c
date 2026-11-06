#include "philo.h"

static int	error(char *message)
{
	int	len;

	len = 0;
	while (message[len])
		len++;
	write(2, "Error: ", 7);
	write(2, message, len);
	write(2, "\n", 1);
	return (1);
}

static void	join_all(t_table *t)
{
	int	i;

	i = 0;
	while (i < t->started)
		pthread_join(t->philos[i++].thread, NULL);
}

static int	start_threads(t_table *t)
{
	int	i;

	t->start = now_ms();
	i = 0;
	while (i < t->count)
		t->philos[i++].last_meal = t->start;
	while (t->started < t->count)
	{
		if (pthread_create(&t->philos[t->started].thread, NULL,
				philo_routine, &t->philos[t->started]))
		{
			set_stop(t);
			join_all(t);
			return (error("cannot create a thread"));
		}
		t->started++;
	}
	return (0);
}

int	main(int argc, char **argv)
{
	t_table	table;
	int		status;

	if (parse_args(argc, argv, &table))
		return (error("usage: ./philo number_of_philosophers time_to_die "
				"time_to_eat time_to_sleep "
				"[number_of_times_each_philosopher_must_eat]\n"
				"       (positive integers, at most 200 philosophers)"));
	if (init_table(&table))
		return (error("initialisation failed"));
	status = start_threads(&table);
	if (status == 0)
	{
		watch(&table);
		join_all(&table);
	}
	destroy_table(&table);
	return (status);
}
