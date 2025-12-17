#!/usr/bin/env python3
import sys
import csv
from collections import deque


class Task:
    def __init__(self, pid, duration, arrival):
        self.pid = pid
        self.left = duration
        self.arrival = arrival


class RoundRobin:
    def __init__(self, tasks, slice_time):
        self.future = deque(tasks)
        self.queue = deque()
        self.slice = slice_time
        self.clock = 0

    def admit_tasks(self):
        while self.future and self.future[0].arrival <= self.clock:
            task = self.future.popleft()
            print(
                f"T={self.clock}: New process {task.pid} "
                f"is waiting for execution (length={task.left})"
            )
            self.queue.append(task)

    def execute(self):
        if self.future:
            print("T=0: No processes currently available")
            self.clock = self.future[0].arrival

        while self.future or self.queue:
            self.admit_tasks()

            if not self.queue:
                self.clock = self.future[0].arrival
                continue

            current = self.queue.popleft()
            work_time = min(self.slice, current.left)
            current.left -= work_time

            print(
                f"T={self.clock}: {current.pid} will be running for "
                f"{work_time} time units. Time left: {current.left}"
            )

            self.clock += work_time
            self.admit_tasks()

            if current.left > 0:
                self.queue.append(current)
            else:
                print(f"T={self.clock}: Process {current.pid} has been finished")

        print(f"T={self.clock}: No more processes in queues")


def parse_csv(file_path):
    result = []
    with open(file_path, newline="") as f:
        for pid, length, start in csv.reader(f):
            result.append(Task(pid, int(length), int(start)))
    return result


def main():
    if len(sys.argv) != 3:
        print("Usage: rr <file.csv> <quantum>")
        sys.exit(1)

    tasks = parse_csv(sys.argv[1])
    quantum = int(sys.argv[2])

    scheduler = RoundRobin(tasks, quantum)
    scheduler.execute()


if __name__ == "__main__":
    main()

