#!/usr/bin/python
import os
import sys
import subprocess

FNULL = open(os.devnull, 'w')
for i in range(1, 11):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (i)
    ret = subprocess.Popen(["ssh", server, "/usr/local/mp1/run.sh"], stdout=FNULL)

