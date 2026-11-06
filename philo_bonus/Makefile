NAME    := philo_bonus
CC      := cc
CFLAGS  := -Wall -Wextra -Werror -pthread
SRCS    := main_bonus.c parse_bonus.c time_bonus.c output_bonus.c \
           sems_bonus.c philosopher_bonus.c
OBJS    := $(SRCS:%.c=obj/%.o)

all: $(NAME)

bonus: all

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

obj/%.o: %.c philo_bonus.h Makefile
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all bonus clean fclean re
