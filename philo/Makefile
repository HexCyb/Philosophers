NAME    := philo
CC      := cc
CFLAGS  := -Wall -Wextra -Werror -pthread
SRCS    := main.c parse.c init.c time.c state.c routine.c monitor.c
OBJS    := $(SRCS:%.c=obj/%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

obj/%.o: %.c philo.h Makefile
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
