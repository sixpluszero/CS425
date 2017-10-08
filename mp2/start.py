import os
import sys

def start(id):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (id)
    os.system("ssh %s \'pkill server; rm -f server*.log; nohup ./mp2/server start > /dev/null 2>&1 & \' " % (server))

for vid in range(1, 4 + 1):
    start(vid)
