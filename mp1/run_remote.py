#!/usr/bin/python
import os
import sys
import subprocess

FNULL = open(os.devnull, 'w')
for i in range(1, 11):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (i)
    os.system("ssh %s \"/usr/local/mp1/run.sh >>server.log\"" % (server))
    print "server %d up" % (i)

