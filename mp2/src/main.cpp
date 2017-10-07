#include "daemon.hpp"
using namespace std;
void foo(){
}
int main(int argc, char* argv[]){
    int flag = 0;
    if (argc > 1) flag = 1;
    Daemon myServer(flag);
    std::thread recv_t (&Daemon::receive, &myServer);
    std::thread hb_t (&Daemon::heartbeat, &myServer);
    std::thread to_t (&Daemon::timeout, &myServer);
    std::thread leave_t (&Daemon::leave, &myServer);
    recv_t.join();
    hb_t.join();
    to_t.join();
    leave_t.join();
}