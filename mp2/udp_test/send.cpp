#include "socketlib.hpp"
using namespace std;

int main(int argc, char *argv[]){
    UDPSocket mySocket = UDPSocket(atoi(argv[1]));
    string msg = "This is UDP socket test";
    mySocket.send("192.17.58.231", msg.c_str());

}
