import socket
import os
import sys
UDP_IP = "127.0.0.1"
RECV_IP = "0.0.0.0"
UDP_PORT = 6666
RECV_PORT = 7777

assert(len(sys.argv) > 1)
MESSAGE = sys.argv[1]

#print "UDP target IP:", UDP_IP
#print "UDP target port:", UDP_PORT
print "message:", MESSAGE



if (MESSAGE == "join"):
    os.system('pkill server')
    os.system('nohup ./mp2/server > /dev/null 2>&1 & ')
elif (MESSAGE == "id"):
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    data, addr = recv_sock.recvfrom(1024)
    print data
    recv_sock.close()
elif (MESSAGE == "member"):
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    data, addr = recv_sock.recvfrom(1024)
    print data
    recv_sock.close()
else:
    recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    recv_sock.bind((RECV_IP, RECV_PORT))
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
    recv_sock.close()

