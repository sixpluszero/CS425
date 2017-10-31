import os
import sys
import mp_config
USER = mp_config.USER

def start(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("ssh %s \'pkill -u %s server; nohup ./mp3/server > /dev/null 2>&1 & \' " % (USER, server))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    start(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        start(vid)
