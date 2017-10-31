import os
import sys
import mp_config
USER = mp_config.USER

def deploy(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("scp -r ../mp3 %s:~/" % (server))
    os.system("ssh %s \' cd mp3; rm -f server.log; make clean; make \' " % (server))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    deploy(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        deploy(vid)
