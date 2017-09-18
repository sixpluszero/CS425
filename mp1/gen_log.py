import os
import sys
import subprocess
import random

all_lines = []

for i in range(1, 11):
    with open("vm%d.log" % (i), "r") as fp:
        for line in fp.readlines():
            all_lines.append(line)

random.shuffle(all_lines)


print os.path.getsize('vm1.log') / (1024 * 1024)


def size_mb(file):
    size = 0
    try:
        size = os.path.getsize(file) / (1024 * 1024)
    except:
        pass
    return size

total_pos = 0
for i in range(1, 5):
    file = "test%d.log" % (i)
    while (size_mb(file) < 60):
        with open(file, "a") as fp:
            for i in range(total_pos, total_pos + 100):
                fp.write(all_lines[i])
        total_pos += 100