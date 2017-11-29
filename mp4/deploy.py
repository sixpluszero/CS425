import os
import sys
import mp_config
from multiprocessing import Pool
USER = mp_config.USER

def rcompile():
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, 1)
    os.system("ssh %s \'pkill -u %s server\'" % (server, USER))
    os.system("scp -r ../mp4 %s:~/" % (server))
    os.system("ssh %s \' cd mp4; rm -f server.log; make clean; make \' " % (server))
    os.system("scp %s:~/mp4/server ./rserv " % (server))
    
def deploy(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("ssh %s \' rm -rf mp4 \' " % (server))
    os.system("scp -r ../mp4 %s:~/" % (server))
    os.system("ssh %s \' rm -f *.log; cd mp4; make clean; make \' " % (server))

assert(len(sys.argv) > 1)
#rcompile()
if (len(sys.argv) == 2):
    deploy(int(sys.argv[1]))
else:
    st = int(sys.argv[1])
    en = int(sys.argv[2])
    p = Pool(en - st + 1)
    li = [x for x in range(st, en + 1)]
    p.map(deploy, li)
