import os
import sys
import mp_config
import time
import socket

assert(len(sys.argv) == 4)
READBUF = 512
TCP_IP = '172.22.154.182'
TCP_PORT = (mp_config.BASEPORT + 3)
BUFFER_SIZE = 1024
MESSAGE = "clientget;%s" % (sys.argv[2])
FNAME = sys.argv[3]

def recv_file(soc):
    total_recv = 100000
    data = s.recv(BUFFER_SIZE)
    dlen = int(data[:10])
    total_recv = dlen - len(data[10:])
    ret = data[10:]
    while (total_recv > 0):
        data = s.recv(BUFFER_SIZE)
        ret = ret + data
        total_recv = total_recv - len(data)
    with open(FNAME,"w") as fp:
        fp.write(ret)


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
if (recv_pack(s) == "ack"):
    recv_file(s)
    print "Received."
else:
    print "File not exist!"