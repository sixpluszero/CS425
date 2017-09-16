import os
import sys
import subprocess

cnt = 0

with open("vm1.log", "r") as fp:
    for line in fp.readlines():
        if line.find("\n") != -1:
            cnt += 1

print cnt