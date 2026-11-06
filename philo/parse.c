#include "philo.h"

/* Strict positive integer: digits only (a leading '+' is fine), 1..INT_MAX. */
static int	to_positive_int(char *s, long *out)
{
	long	value;

	value = 0;
	if (*s == '+')
		s++;
	if (*s == '\0')
		return (1);
	while (*s >= '0' && *s <= '9')
	{
		value = value * 10 + (*s - '0');
		if (value > 2147483647)
			return (1);
		s++;
	}
	if (*s != '\0' || value == 0)
		return (1);
	*out = value;
	return (0);
}

/*
** With an odd number of philosophers somebody always waits a whole meal for
** a fork, so each one thinks long enough to let its neighbours eat first.
*/
static long	think_time(t_table *t)
{
	long	think;

	if (t->count % 2 == 0)
		return (0);
	think = t->time_to_eat * 2 - t->time_to_sleep;
	if (think < 0)
		return (0);
	return (think);
}

int	parse_args(int argc, char **argv, t_table *t)
{
	long	v[5];
	int		i;

	if (argc != 5 && argc != 6)
		return (1);
	v[4] = -1;
	i = 1;
	while (i < argc)
	{
		if (to_positive_int(argv[i], &v[i - 1]))
			return (1);
		i++;
	}
	if (v[0] > MAX_PHILOS)
		return (1);
	memset(t, 0, sizeof(*t));
	t->count = (int)v[0];
	t->time_to_die = v[1];
	t->time_to_eat = v[2];
	t->time_to_sleep = v[3];
	t->must_eat = (int)v[4];
	t->think_time = think_time(t);
	return (0);
}
