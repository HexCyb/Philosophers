# Philosophers

[![CI](https://github.com/HexCyb/Philosophers/actions/workflows/ci.yml/badge.svg)](https://github.com/HexCyb/Philosophers/actions/workflows/ci.yml)

The dining philosophers problem in C, twice: with **threads and mutexes** (`philo`) and with
**processes and semaphores** (`philo_bonus`). No data races, no deadlock, and a death is announced
less than 10 ms after it happens.

> Originally completed at **42 Türkiye** as part of the common core. This repository is a 2026 rewrite.
> 42 Türkiye'de tamamladığım projenin 2026'da yeniden yazılmış hali.

## Usage

```sh
make                                   # builds philo/philo and philo_bonus/philo_bonus
./philo/philo 5 800 200 200            # philosophers, time_to_die, time_to_eat, time_to_sleep (ms)
./philo/philo 5 800 200 200 7          # stops once everybody has eaten 7 times
./philo_bonus/philo_bonus 5 800 200 200
make test                              # behaviour tests for both programs
make tsan                              # rebuild with ThreadSanitizer and look for data races
```

```
0 1 has taken a fork
0 1 has taken a fork
0 1 is eating
0 2 is thinking
...
801 1 died
```

Every value must be a positive integer (at most 200 philosophers); anything else prints `Error: usage …`
and exits with 1.

## philo: threads and mutexes

- **No deadlock.** A deadlock needs everyone to hold one fork and wait for the next one in the same
  direction. Odd philosophers take their left fork first and even philosophers their right fork, so that
  circle cannot close.
- **No starvation.** Even philosophers start by thinking for half a meal, so the odd ones eat first instead
  of everyone grabbing one fork. With an odd number of philosophers somebody always waits a whole meal,
  so each philosopher thinks for `2 × time_to_eat − time_to_sleep` ms before reaching for forks again
  (`5 610 200 200` keeps everybody alive).
- **On-time deaths.** The main thread checks every philosopher about every 0.5 ms. `last_meal` and
  `meals` are written by the philosopher and read by the monitor, so each philosopher has a `meal_lock`.
- **Clean output.** A line is printed while holding `print_lock`, and the timestamp is taken inside it, so
  lines never mix and time never goes backwards. The monitor sets the stop flag *before* printing `died`,
  under the same lock, so no line can follow a death.
- **Precise sleeping.** `usleep` oversleeps a little on every call. `nap()` sleeps half of the remaining
  time, then short steps, which keeps meals on schedule even with 200 threads.

## philo_bonus: processes and semaphores

- Every philosopher is a process; the forks lie in the middle of the table, as one counting semaphore.
- A `seats` semaphore lets at most `count / 2` philosophers reach for forks at the same time, so they can
  never all hold one fork each.
- Each process runs a small watcher thread for its own philosopher. On a death it takes the `print`
  semaphore, prints `died` and exits *without* giving it back: nobody can print after it. The parent sees
  the exit status and kills the others.
- A philosopher who has eaten enough stops its watcher, joins it and exits with 0; when every child has
  exited with 0, the simulation is over.
- Semaphores are unlinked right after `sem_open`: nothing is left in `/dev/shm`, even after `kill -9`.
- Lines are written with a single `write()` (no stdio buffer), so a killed process cannot lose lines.

## Tests

`tests/run_tests.py` runs both programs through a pseudo-terminal and checks the log, line by line:

| Scenario | Expected |
|---|---|
| `1 800 200 200` | one fork, then `died` at 800–810 ms |
| `4 310 200 100`, `3 300 200 200` | exactly one death, at most 10 ms late, last line |
| `5 800 200 200`, `5 610 200 200`, `4 410 200 200`, `2 400 100 100`, `200 800 200 200` | nobody dies, everybody eats |
| `5 800 200 200 7`, `4 410 200 200 10` | exits by itself, everybody ate enough |
| 12 kinds of bad input | `Error…` on stderr, exit 1, nothing on stdout |

For every log it also checks that each meal starts with two forks, that neighbours never eat at the same
time (`philo`), that at most `count / 2` philosophers eat at once, and that timestamps never go backwards.

[CI](.github/workflows/ci.yml) builds with gcc and clang and runs the tests, rebuilds with ThreadSanitizer
(no data race reported), and runs both programs under Valgrind (no leak, children included).
