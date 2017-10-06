#include "daemon.hpp"
using namespace std;
void foo(){
}
int main(int argc, char* argv[]){
    int flag = 0;
    if ((argc > 1) && (strcmp(argv[1], "prime_start") == 0)) flag = 1;
    Daemon myServer(flag);
    std::thread recv_t (&Daemon::receive, &myServer);
    //myServer.receive();
    
    recv_t.join();
}