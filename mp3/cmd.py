import socket
import os
import sys
import mp_config
BASEPORT = mp_config.BASEPORT
USER = mp_config.USER
CMD_IP = "127.0.0.1"
RECV_IP = "0.0.0.0"
CMD_PORT = (BASEPORT+2)
RECV_PORT = BASEPORT

assert(len(sys.argv) > 1)
MESSAGE = sys.argv[1]

print "message:", MESSAGE

if (MESSAGE == "join"):
    os.system('pkill -u %s server' % (USER))
    os.system('nohup ./mp3/server > /dev/null 2>&1 & ')
elif (MESSAGE == "id"):
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (CMD_IP, CMD_PORT))
    data, addr = recv_sock.recvfrom(1024)
    print data
    recv_sock.close()
elif (MESSAGE == "member"):
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (CMD_IP, CMD_PORT))
    data, addr = recv_sock.recvfrom(1024)
    print data
    recv_sock.close()
else:
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (CMD_IP, CMD_PORT))
    recv_sock.close()

