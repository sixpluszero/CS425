#ifndef MP2_SOCKET_HPP
#define MP2_SOCKET_HPP

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <string>

#define BUFSIZE 2048
//#define PORT 27015 // This is Counter Strike 1.6's port.

class UDPSocket {
private:
    struct sockaddr_in serverAddr, clientAddr, sendAddr, recvAddr;
    int serverFD, clientFD, port;
public:
    UDPSocket(int port);
    UDPSocket(int port, bool send);
    void setTimeout(int time);
    int send(const char* remoteIP, const char* msg);
    int recv(char* remoteIp, char* recvMsg);    
};




#endif
