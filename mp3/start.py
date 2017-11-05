import os
import sys
import mp_config
import time
USER = mp_config.USER

def start(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("ssh %s \'pkill -u %s server; rm -f *.log; nohup ./mp3/server > /dev/null 2>&1 & \' " % (server, USER))
    #os.system("ssh %s \'pkill -u %s server; rm -f server*.log; nohup ./mp3/server > /dev/null 2>&1 & \' " % (server, USER))

assert(len(sys.argv) == 3 or len(sys.argv) == 1)

if (len(sys.argv) == 3):
    for vid in range(int(sys.argv[1]), int(sys.argv[2])+1):
        start(vid)
        time.sleep(1)
else:
    for vid in range(1, 10+1):
        start(vid)
