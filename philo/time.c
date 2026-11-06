#include "philo.h"

long	now_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000L + tv.tv_usec / 1000);
}

static long	now_us(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000L + tv.tv_usec);
}

/*
** usleep() alone oversleeps by a few hundred microseconds every call, and
** those errors add up meal after meal. Sleeping half of what is left keeps
** the number of wake-ups small and the end time precise. It also returns
** early once the simulation is over.
*/
void	nap(t_table *table, long ms)
{
	long	end;
	long	left;

	end = now_us() + ms * 1000;
	left = end - now_us();
	while (left > 0 && !is_stopped(table))
	{
		if (left > 2000)
			usleep(left / 2);
		else if (left > 100)
			usleep(100);
		else
			usleep(left);
		left = end - now_us();
	}
}
