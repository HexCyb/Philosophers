#include "philo_bonus.h"

static int	put_nbr(char *buf, int len, long n)
{
	char	digits[20];
	int		i;

	i = 0;
	if (n <= 0)
		digits[i++] = '0';
	while (n > 0)
	{
		digits[i++] = '0' + n % 10;
		n /= 10;
	}
	while (i > 0)
		buf[len++] = digits[--i];
	return (len);
}

/*
** One write() per line and no stdio buffer: a process that gets killed
** cannot take half-printed lines with it.
*/
void	put_status(long ms, int id, char *message)
{
	char	buf[64];
	int		len;

	len = put_nbr(buf, 0, ms);
	buf[len++] = ' ';
	len = put_nbr(buf, len, id);
	buf[len++] = ' ';
	while (*message && len < 62)
		buf[len++] = *message++;
	buf[len++] = '\n';
	write(1, buf, len);
}

void	say(t_table *t, char *message)
{
	sem_wait(t->print);
	put_status(now_ms() - t->start, t->id, message);
	sem_post(t->print);
}

int	error(char *message)
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

/* "/philo_<kind>" or "/philo_<kind>_<id>" when id > 0 */
void	sem_name(char *buf, char *kind, int id)
{
	int		len;
	char	*prefix;

	prefix = "/philo_";
	len = 0;
	while (*prefix)
		buf[len++] = *prefix++;
	while (*kind && len < 20)
		buf[len++] = *kind++;
	if (id > 0)
	{
		buf[len++] = '_';
		len = put_nbr(buf, len, id);
	}
	buf[len] = '\0';
}
