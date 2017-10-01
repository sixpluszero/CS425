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
#include <string>

#define BUFSIZE 2048
#define PORT 27015 // This is Counter Strike 1.6's port.

class UDPSocket {
private:
    struct sockaddr_in serverAddr, clientAddr, sendAddr, recvAddr;
    int serverFD, clientFD;
public:
    UDPSocket();
    int Send(const char* remoteIP, const char* msg);
    int Recv(char* remoteIp, char* recvMsg);
    
};




#endif
