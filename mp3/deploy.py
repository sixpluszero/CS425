import os
import sys
import mp_config
USER = mp_config.USER

def rcompile():
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, 1)
    os.system("ssh %s \'pkill -u %s server\'" % (server, USER))
    os.system("scp -r ../mp3 %s:~/" % (server))
    os.system("ssh %s \' cd mp3; rm -f server.log; make clean; make \' " % (server))
    os.system("scp %s:~/mp3/server ./rserv " % (server))   
    
    
def deploy(id):
    server = "%s@fa17-cs425-g59-%02d.cs.illinois.edu" % (USER, id)
    os.system("scp -r ../mp3 %s:~/" % (server))
    os.system("ssh %s \' cd mp3; rm -f server.log; make clean; make \' " % (server))
    #os.system("ssh %s \'pkill -u %s server; mkdir mp3; mkdir mp3/files \'" % (server, USER))
    #os.system("scp ./rserv %s:~/mp3/server " % (server))

assert(len(sys.argv) > 1)
#rcompile()
if (len(sys.argv) == 2):
    deploy(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        deploy(vid)
