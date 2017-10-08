import os
import sys

def start(id):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (id)
    os.system("ssh %s \'pkill server; nohup ./mp2/server > /dev/null 2>&1 & \' " % (server))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    start(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        start(vid)
