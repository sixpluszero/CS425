#include "udp_test.hpp"
using namespace std;

int main(){
    UDPSocket mySocket = UDPSocket();
    string msg = "This is UDP socket test";
    mySocket.Send("172.22.154.182", msg.c_str());

}