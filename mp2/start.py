import os
import sys

def start(id):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (id)
    os.system("ssh %s \'pkill server; rm -f server*.log; nohup ./mp2/server start > /dev/null 2>&1 & \' " % (server))

if (len(sys.argv) == 3):
    for vid in range(int(sys.argv[1]), int(sys.argv[2])+1):
        start(vid)
else:
    for vid in range(1, 10+1):
        start(vid)
