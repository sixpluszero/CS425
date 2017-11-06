import os
import sys
import time
import socket
import thread
import signal
import threading
import mp_config
BASEPORT = mp_config.BASEPORT
BUFFER_SIZE = 4096
BIGSIZE = 1000000

def pad(w):
    s = str(w)
    for i in range(0, 10-len(s)):
        s = s + " "
    return s

def send_pack(soc, s):
    w = str(len(s))
    for i in range(0, 10-len(w)):
        s = " " + s
    s = w + s
    soc.send(s)

def recv_pack(soc):
    try:
        total_recv = BIGSIZE
        data = soc.recv(BUFFER_SIZE)
        dlen = int(data[:10])
        total_recv = dlen - len(data[10:])
        ret = data[10:]
        while (total_recv > 0):
            data = soc.recv(BUFFER_SIZE)
            ret = ret + data
            total_recv = total_recv - len(data)
        return ret
    except:
        print "Remote data error. Exit"
        exit()

def recv_file(soc, fname):
    total_recv = 0
    total_size = BIGSIZE
    first = True
    with open(fname, "wb") as fp:
        while total_recv < total_size:
            data = soc.recv(BUFFER_SIZE)
            if (first == True):
                total_size = int(data[:10])
                data = data[10:] 
                first = False
            fp.write(data)
            total_recv += len(data)
    print total_recv

def send_file(soc, fname):
    fsize = os.stat(fname).st_size
    soc.send(pad(fsize))
    total_send = 0
    with open(fname, "rb") as fp:
        while total_send < fsize + 10:
            substr = fp.read(BUFFER_SIZE)
            if not substr:
                break
            soc.send(substr)
            total_send += len(substr)
        print fsize, total_send

def connect_master():
    LOCALHOST = '0.0.0.0'
    for i in range(10):
        UDP_IP = '172.22.154.' + str(182+i)
        RECV_PORT = int(BASEPORT)+2
        CMD_PORT = int(BASEPORT)
        try:
            recv_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            recv_sock.bind((LOCALHOST, CMD_PORT))        
            recv_sock.settimeout(1)
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.sendto("query", (UDP_IP, RECV_PORT))
            data, _ = recv_sock.recvfrom(1024)
        except:
            recv_sock.close()
            continue
        if (data == "yes"):
            return UDP_IP
        recv_sock.close()
    return "fail"

def TCPConnect(TCP_IP, TCP_PORT):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((TCP_IP, TCP_PORT))
    except:
        return None
    return s

def command_check(args):
    if (len(args) == 1):
        print "No command detected. Exit"
        exit(0)
    if (not args[1] in ["put", "get", "delete", "ls"]):
        print "Command %s not supported. Exit" % (args[1])
        exit(0)

class AlarmException(Exception):
    pass

def alarmHandler(signum, frame):
    raise AlarmException

def main(args):
    # First check if command is recognized. If not, return
    command_check(args)
    data = connect_master()
    if (data == "fail"):
        print "Error: Cannot connect to server"
        return
    else:
        print "Master IP is:", data
    TCP_IP = data
    TCP_PORT = int(BASEPORT) + 3
    soc = TCPConnect(TCP_IP, TCP_PORT)
    if soc == None:
        print "Error: TCP connection error"
        return
    assert(len(args) > 1)
    if args[1] == "put":
        assert(len(args) == 4)
        MESSAGE = "clientput;" + args[3]
        send_pack(soc, MESSAGE)
        data = recv_pack(soc)
        if (data != "ack"):
            uin = "No"
            signal.signal(signal.SIGALRM, alarmHandler)
            signal.alarm(30)
            try:
                uin = raw_input("File %s exists, do you want to update? Y/n\n" % (args[3]))
                signal.alarm(0)
            except:
                print "No keyboard input in 30 seconds. Rejecting this update"
                uin = "No"
            if (uin in ["Y", "Yes", "y", "yes"]):
                send_pack(soc, "Yes")
                data = recv_pack(soc)
                print data
                send_file(soc, args[2])
                data = recv_pack(soc)
                print data
                if (data == "success"):
                    print "Put success"
                else:
                    print "Put failed:" + data
                soc.close()
            else:
                send_pack(soc, "No")
                soc.close()
                return
        else:
            send_file(soc, args[2])
            data = recv_pack(soc)
            if (data == "success"):
                print "Put success"
            else:
                print "Put failed:" + data
            soc.close()
    elif args[1] == "get":
        assert(len(args) == 4)
        MESSAGE = "clientget;" + args[2]
        send_pack(soc, MESSAGE)
        data = recv_pack(soc)
        if (data == "ack"):
            print "receiving data"
            send_pack(soc, "ack")
            recv_file(soc, args[3])
        else:
            print "File not exists in remote SDFS"
        soc.close()
    
    elif args[1] == "delete":
        assert(len(args) == 3)
        MESSAGE = "clientdel;" + args[2]
        send_pack(soc, MESSAGE)
        data = recv_pack(soc)
        if (data == "ack"):
            print "File deleted"
        else:
            print "Delete error:", data
        soc.close()
    elif args[1] == "ls":
        assert(len(args) == 3)
        MESSAGE = "clientlist;" + args[2]
        send_pack(soc, MESSAGE)
        data = recv_pack(soc)
        if (data == "file not exists"):
            print "File not exist"
        elif (data == "fail"):
            print "Other error"
        else:
            print "File %s stores at %s" % (args[2], data)
        soc.close()
    else:
        print "command not recognized"
        soc.close()

main(sys.argv)