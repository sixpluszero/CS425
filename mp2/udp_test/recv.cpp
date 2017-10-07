#include "socketlib.hpp"
using namespace std;

int main(){
    UDPSocket mySocket(50051);
    char remoteIP[100];
    char buf[2048];
    int recvBytes;
    recvBytes = mySocket.recv(remoteIP, buf);
    printf("Receive %d bytes from %s\n%s\n", recvBytes, remoteIP, buf);
}