#include "udp_test.hpp"
using namespace std;

int main(){
    UDPSocket mySocket = UDPSocket();
    char remoteIP[100];
    char buf[2048];
    int recvBytes;
    recvBytes = mySocket.Recv(remoteIP, buf);
    printf("Receive %d bytes from %s\n%s\n", recvBytes, remoteIP, buf);
}