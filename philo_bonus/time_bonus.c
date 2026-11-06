#include "philo_bonus.h"

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

/* Sleep half of what is left, then short steps: precise and cheap. */
void	nap(long ms)
{
	long	end;
	long	left;

	end = now_us() + ms * 1000;
	left = end - now_us();
	while (left > 0)
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
