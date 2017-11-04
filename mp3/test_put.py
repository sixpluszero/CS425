import os
import sys
import mp_config
import time
import socket

assert(len(sys.argv) == 4)
READBUF = 4096
TCP_IP = '172.22.154.182'
TCP_PORT = (mp_config.BASEPORT + 3)
BUFFER_SIZE = 1024
MESSAGE = "clientput;%s" % (sys.argv[3])
FNAME = sys.argv[2]

def send_file(soc, fname):
    fsize = os.stat(fname).st_size
    print fsize
    total_send = 0
    with open(fname, "r") as fp:
        while total_send < fsize + 10:
            substr = fp.read(READBUF)
            if (total_send == 0):
                soc.send(pad(fsize) + substr)
                total_send += (len(substr) + 10)
            else:
                soc.send(substr)
                total_send += len(substr)
        print total_send - 10

def send_pack(soc, s):
    w = str(len(s))
    for i in range(0, 10-len(w)):
        s = " " + s
    s = w + s
    soc.send(s)

def pad(w):
    s = str(w)
    for i in range(0, 10-len(s)):
        s = s + " "
    return s

def recv_pack(soc):
    total_recv = 100000
    data = s.recv(BUFFER_SIZE)
    dlen = int(data[:10])
    total_recv = dlen - len(data[10:])
    ret = data[10:]
    while (total_recv > 0):
        data = s.recv(BUFFER_SIZE)
        ret = ret + data
        total_recv = total_recv - len(data)
    return ret

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
send_pack(s, MESSAGE)
data = recv_pack(s)
print data
if (data != "ack"):
    uin = raw_input("Please confirm if you want to go on: Y/n\n")
    if (uin in ["Y", "Yes", "y", "yes"]):
        send_pack(s, "Yes")
        send_file(s, FNAME)
        data = recv_pack(s)
        print data
    else:
        send_pack(s, "No")
        s.close()
else:
    print "sending file"
    send_file(s, FNAME)
    print "file sent"
    data = recv_pack(s)
    print "write succeed"
    s.close()
    print data