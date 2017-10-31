#include "socketlib.hpp"

UDPSocket::UDPSocket(int _port) {
    port = _port;
    /* create server socket */
    if ((serverFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create server socket\n");
    }

    /* bind server socket with fd */   
    memset((char *)&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);

    if (bind(serverFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind server socket failed");
    }

    /* create client socket */

    if ((clientFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create client socket\n");
    }

    /* bind it to all local addresses and pick any port number */

    memset((char *)&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientFD, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("bind client socket failed");
    }
    printf("%d binding finished\n", port); 
}

UDPSocket::UDPSocket(int _port, bool _send) {
    port = _port;

    /* create client socket */

    if ((clientFD = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create client socket\n");
    }

    /* bind it to all local addresses and pick any port number */

    memset((char *)&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientFD, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("bind client socket failed");
    }
    printf("%d binding finished\n", port); 
}

int UDPSocket::send(const char * remoteIP, const char * msg) {
    memset((char *) &sendAddr, 0, sizeof(sendAddr));
	sendAddr.sin_family = AF_INET;
    sendAddr.sin_port = htons(port);
    inet_pton(AF_INET, remoteIP, &(sendAddr.sin_addr));
    if (sendto(clientFD, msg, strlen(msg), 0, (struct sockaddr *)&sendAddr, sizeof(sendAddr)) < 0) {
        printf("sendto: %s %d, %s\n", remoteIP, port, msg);
        perror("sendto");
        return -1;
    }
    return 0;
}

int UDPSocket::recv(char * remoteIP, char * recvMsg) {
    int recvlen;
    socklen_t recvAddrLen = sizeof(recvAddr);
    recvlen = recvfrom(serverFD, recvMsg, BUFSIZE, 0, (struct sockaddr *)&recvAddr, &recvAddrLen);
    inet_ntop(AF_INET, &(recvAddr.sin_addr), remoteIP, INET_ADDRSTRLEN);
    recvMsg[recvlen] = '\0';
    return recvlen;
}

void UDPSocket::setTimeout(int time) {    
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = time;
    if (setsockopt(serverFD, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
    }
}