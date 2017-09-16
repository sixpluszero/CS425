import os
import sys
import subprocess



for i in range(10):
    server = "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (i + 1)
    cmd = "chmod 777 run.sh"
    os.system("scp dgrep.cpp worker.cpp run.sh %s:~/" % (server))
    ret = subprocess.Popen(["ssh", server, cmd])
