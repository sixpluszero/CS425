import os
import sys
import mp_config
USER = mp_config.USER

def crash(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("ssh %s \'pkill -u %s server\'" % (server, USER))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    crash(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        crash(vid)
