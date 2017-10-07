import os
import sys

def crash(id):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (id)
    os.system("ssh %s \'pkill server\'" % (server))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    crash(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        crash(vid)
