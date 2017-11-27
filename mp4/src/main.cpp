#include "daemon.hpp"
using namespace std;

int main(int argc, char* argv[]){
  int flag = 0;
  if (argc > 1) flag = 1;
  Daemon myServer(flag);
  std::thread recv_t (&Daemon::receive, &myServer);
  std::thread hb_t (&Daemon::heartbeat, &myServer);
  std::thread to_t (&Daemon::timeout, &myServer);
  std::thread cmd_t (&Daemon::command, &myServer);
  std::thread nc_t (&Daemon::sdfs, &myServer);
  std::thread sa_t (&Daemon::sava, &myServer);
  hb_t.join();
  exit(0);
  recv_t.join();    
  to_t.join();
  cmd_t.join();
  nc_t.join();
  sa_t.join();
}