# Each program has its own Makefile (philo/ and philo_bonus/), as the subject asks.
# This one only builds both and runs the tests.

all:
	$(MAKE) -C philo
	$(MAKE) -C philo_bonus

test: all
	python3 tests/run_tests.py

# Rebuilds both with ThreadSanitizer and checks that it reports no data race.
tsan:
	$(MAKE) -C philo re CFLAGS="-Wall -Wextra -Werror -pthread -g -fsanitize=thread"
	$(MAKE) -C philo_bonus re CFLAGS="-Wall -Wextra -Werror -pthread -g -fsanitize=thread"
	python3 tests/run_tests.py --tsan

clean:
	$(MAKE) -C philo clean
	$(MAKE) -C philo_bonus clean

fclean:
	$(MAKE) -C philo fclean
	$(MAKE) -C philo_bonus fclean

re: fclean all

.PHONY: all test tsan clean fclean re
