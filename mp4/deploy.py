import os
import sys
import mp_config
USER = mp_config.USER

def rcompile():
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, 1)
    os.system("ssh %s \'pkill -u %s server\'" % (server, USER))
    os.system("scp -r ../mp4 %s:~/" % (server))
    os.system("ssh %s \' cd mp4; rm -f server.log; make clean; make \' " % (server))
    os.system("scp %s:~/mp4/server ./rserv " % (server))   
    
    
def deploy(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("scp -r ../mp4 %s:~/" % (server))
    os.system("ssh %s \' rm -f *.log; cd mp4; make clean; make \' " % (server))
    #os.system("ssh %s \'pkill -u %s server; mkdir mp4; mkdir mp4/files \'" % (server, USER))
    #os.system("scp ./rserv %s:~/mp4/server " % (server))

assert(len(sys.argv) > 1)
#rcompile()
if (len(sys.argv) == 2):
    deploy(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        deploy(vid)
