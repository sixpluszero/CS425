import os
import sys
import socket
import mp_config
BASEPORT = mp_config.BASEPORT
def leave(id):
    UDP_IP = "172.22.154.%d" % (id + 181)
    UDP_PORT = (BASEPORT+2)
    MESSAGE = "leave"
    print "UDP target IP:", UDP_IP
    print "UDP target port:", UDP_PORT
    print "message:", MESSAGE
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))

assert(len(sys.argv) > 1)
if (len(sys.argv) == 2):
    leave(int(sys.argv[1]))
else:
    for vid in range(int(sys.argv[1]), int(sys.argv[2]) + 1):
        leave(vid)
