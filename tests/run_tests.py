#!/usr/bin/env python3
"""Black-box tests for philo (threads + mutexes) and philo_bonus (processes + semaphores).

Every run goes through a pseudo-terminal, like in a shell: stdout is line buffered, so nothing is
lost when a simulation that should run forever is stopped by the test.

    python3 tests/run_tests.py              # both programs, normal build
    python3 tests/run_tests.py --tsan       # ThreadSanitizer build: only checks that no data race is reported
"""
import os
import pty
import re
import select
import signal
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LINE = re.compile(r"^(\d+) (\d+) (has taken a fork|is eating|is sleeping|is thinking|died)$")
FAILURES = []


def run(binary, args, timeout):
    """Run the program for at most `timeout` seconds. Returns (lines, exit code or None, stderr)."""
    master, slave = pty.openpty()
    proc = subprocess.Popen([str(binary), *map(str, args)], stdout=slave, stderr=subprocess.PIPE,
                            start_new_session=True)
    os.close(slave)
    deadline = time.monotonic() + timeout
    chunks = []
    finished = True
    while True:
        left = deadline - time.monotonic()
        if left <= 0:
            finished = False
            break
        ready, _, _ = select.select([master], [], [], min(left, 0.1))
        if not ready:
            continue
        try:
            data = os.read(master, 65536)
        except OSError:  # EIO: every writer closed the terminal
            break
        if not data:
            break
        chunks.append(data)
    if not finished:
        os.killpg(proc.pid, signal.SIGKILL)  # the bonus' children too
    proc.wait()
    os.close(master)
    stderr = proc.stderr.read().decode(errors="replace")
    text = b"".join(chunks).decode(errors="replace").replace("\r", "")
    return text.splitlines(), (proc.returncode if finished else None), stderr


def check(name, condition, detail=""):
    if condition:
        print(f"  ok    {name}")
    else:
        print(f"  FAIL  {name}  {detail}")
        FAILURES.append(name)


def parse(lines, count):
    """Validates the log format and returns [(ms, id, action)]."""
    events = []
    for line in lines:
        match = LINE.match(line)
        if not match:
            return None, f"bad line: {line!r}"
        ms, who, action = int(match[1]), int(match[2]), match[3]
        if not 1 <= who <= count:
            return None, f"unknown philosopher: {line!r}"
        events.append((ms, who, action))
    for before, after in zip(events, events[1:]):
        if after[0] < before[0]:
            return None, f"time goes backwards: {before} then {after}"
    return events, ""


def rules_hold(events, count, eat, bonus):
    """Forks before every meal, nothing after a death, never too many (or neighbouring) eaters.

    A meal lasts time_to_eat from its "is eating" line. Its end is not the next "is sleeping" line,
    because a neighbour may grab the fork and print before that line.
    """
    forks = {i: 0 for i in range(1, count + 1)}
    meals = []
    for index, (ms, who, action) in enumerate(events):
        if action == "died":
            if index != len(events) - 1:
                return f"line after the death: {events[index + 1]}"
        elif action == "has taken a fork":
            forks[who] += 1
            if forks[who] > 2:
                return f"philosopher {who} holds more than two forks at {ms}"
        elif action == "is eating":
            if forks[who] != 2:
                return f"philosopher {who} eats with {forks[who]} fork(s) at {ms}"
            meals.append((ms, who))
        elif action == "is sleeping":
            forks[who] = 0
    eating = []
    for ms, who in meals:
        eating = [(end, p) for end, p in eating if end > ms + 1]  # 1 ms of rounding slack
        if len(eating) + 1 > count // 2:
            return f"{len(eating) + 1} philosophers eat at {ms} with {count} forks"
        neighbours = {(who % count) + 1, ((who - 2) % count) + 1} - {who}
        if not bonus and any(p in neighbours for _, p in eating):
            return f"philosopher {who} eats at {ms} while a neighbour is eating"
        eating.append((ms + eat, who))
    return ""


def last_meal(events, who, death_ms):
    meals = [ms for ms, p, a in events if p == who and a == "is eating" and ms <= death_ms]
    return meals[-1] if meals else 0


def scenario_dies(binary, bonus, args, precise=True):
    count, die = args[0], args[1]
    lines, code, stderr = run(binary, args, timeout=5)
    events, why = parse(lines, count)
    name = f"{' '.join(map(str, args))}: somebody dies"
    if events is None:
        return check(name, False, why)
    deaths = [e for e in events if e[2] == "died"]
    check(name, code is not None and len(deaths) == 1, f"exit={code} deaths={deaths[-3:]}")
    if len(deaths) == 1:
        ms, who, _ = deaths[0]
        late = ms - last_meal(events, who, ms) - die
        if precise:
            check(f"{' '.join(map(str, args))}: death announced within 10 ms", 0 <= late <= 10,
                  f"{late} ms after time_to_die")
    problem = rules_hold(events, count, args[2], bonus)
    check(f"{' '.join(map(str, args))}: rules hold", not problem, problem)
    return stderr


def scenario_survives(binary, bonus, args, seconds):
    count = args[0]
    lines, code, stderr = run(binary, args, timeout=seconds)
    events, why = parse(lines, count)
    name = f"{' '.join(map(str, args))}: nobody dies in {seconds} s"
    if events is None:
        return check(name, False, why)
    deaths = [e for e in events if e[2] == "died"]
    meals = {p: 0 for p in range(1, count + 1)}
    for _, who, action in events:
        if action == "is eating":
            meals[who] += 1
    check(name, code is None and not deaths, f"exit={code} death={deaths[:1]}")
    check(f"{' '.join(map(str, args))}: everybody eats", min(meals.values()) >= 2,
          f"fewest meals: {min(meals.values())}")
    problem = rules_hold(events, count, args[2], bonus)
    check(f"{' '.join(map(str, args))}: rules hold", not problem, problem)
    return stderr


def scenario_must_eat(binary, bonus, args):
    count, goal = args[0], args[4]
    lines, code, stderr = run(binary, args, timeout=10)
    events, why = parse(lines, count)
    name = f"{' '.join(map(str, args))}: stops once everybody ate {goal} times"
    if events is None:
        return check(name, False, why)
    meals = {p: 0 for p in range(1, count + 1)}
    for _, who, action in events:
        if action == "is eating":
            meals[who] += 1
    deaths = [e for e in events if e[2] == "died"]
    check(name, code == 0 and not deaths and min(meals.values()) >= goal,
          f"exit={code} deaths={deaths[:1]} meals={meals}")
    problem = rules_hold(events, count, args[2], bonus)
    check(f"{' '.join(map(str, args))}: rules hold", not problem, problem)
    return stderr


def invalid_input(binary):
    cases = [[], [5, 800, 200], [5, 800, 200, 200, 7, 1], [5, 800, 200, "abc"], [-5, 800, 200, 200],
             [0, 800, 200, 200], [5, 800, 200, 200, 0], [201, 800, 200, 200], [5, 2147483648, 200, 200],
             [5, "", 200, 200], [5, "80 0", 200, 200], [5, "800ms", 200, 200]]
    for args in cases:
        lines, code, stderr = run(binary, args, timeout=3)
        check(f"refuses {args}", code == 1 and not lines and stderr.startswith("Error"),
              f"exit={code} stdout={lines[:2]} stderr={stderr[:40]!r}")


def tsan_clean(name, stderr):
    check(f"{name}: no data race reported", "ThreadSanitizer" not in stderr, stderr[:400])


def main():
    tsan = "--tsan" in sys.argv
    programs = [(ROOT / "philo" / "philo", False), (ROOT / "philo_bonus" / "philo_bonus", True)]
    for binary, bonus in programs:
        if not binary.exists():
            sys.exit(f"{binary} is missing: build it first")
    for binary, bonus in programs:
        print(f"\n== {binary.name}{' (ThreadSanitizer)' if tsan else ''}")
        if tsan:
            tsan_clean("5 800 200 200 3", scenario_must_eat(binary, bonus, [5, 800, 200, 200, 3]))
            tsan_clean("4 310 200 100", scenario_dies(binary, bonus, [4, 310, 200, 100], precise=False))
            tsan_clean("1 800 200 200", scenario_dies(binary, bonus, [1, 800, 200, 200], precise=False))
            continue
        invalid_input(binary)
        scenario_dies(binary, bonus, [1, 800, 200, 200])
        scenario_dies(binary, bonus, [4, 310, 200, 100])
        scenario_dies(binary, bonus, [3, 300, 200, 200])
        scenario_survives(binary, bonus, [5, 800, 200, 200], 5)
        scenario_survives(binary, bonus, [5, 610, 200, 200], 5)
        scenario_survives(binary, bonus, [4, 410, 200, 200], 5)
        scenario_survives(binary, bonus, [2, 400, 100, 100], 3)
        scenario_survives(binary, bonus, [200, 800, 200, 200], 3)
        scenario_must_eat(binary, bonus, [5, 800, 200, 200, 7])
        scenario_must_eat(binary, bonus, [4, 410, 200, 200, 10])
    print()
    if FAILURES:
        print(f"{len(FAILURES)} check(s) failed")
        sys.exit(1)
    print("all checks passed")


if __name__ == "__main__":
    main()
