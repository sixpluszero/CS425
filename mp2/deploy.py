import os
import sys

def deploy(id):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (id)
    os.system("scp -r ../mp2 %s:~/" % (server))
    os.system("ssh %s \' cd mp2; make clean; make \' " % (server))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    deploy(int(sys.argv[1]))
else:
    for id in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        deploy(id)
