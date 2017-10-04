#include "daemon.hpp"
using namespace std;


int main(int argc, char* argv[]){
    if (argc > 1) {
        if (strcmp(argv[1], "prime_start") == 0) {
            Daemon myServer(1);
            myServer.receive();
        } else {
            Daemon myServer(0);
        }

        

    }
}