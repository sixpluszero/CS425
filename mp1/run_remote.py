import os
import sys
import subprocess

'''
for i in range(10):
    os.system("wget https://courses.engr.illinois.edu/cs425/fa2017/CS425_MP1_Demo_Logs_FA17/vm%d.log" % (i+1))
'''

'''
for i in range(10):
    os.system("scp vm%d.log jialin2@fa17-cs425-g59-%02d.cs.illinois.edu:~/" % (i+1, i+1))
'''

'''
for i in range(2):
    os.system("scp worker.cpp run.sh jialin2@fa17-cs425-g59-%02d.cs.illinois.edu:~/" % (i+1))
    ret = subprocess.Popen(["ssh", "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (i+1), "chmod 777 run.sh"])
'''

FNULL = open(os.devnull, 'w')
for i in range(10):
    ret = subprocess.Popen(["ssh", "jialin2@fa17-cs425-g59-%02d.cs.illinois.edu" % (i+1), "./run.sh"], stdout=FNULL)

