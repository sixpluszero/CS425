#include "daemon.hpp"
using namespace std;

Daemon::Daemon(int flag): msg_socket(UDPSocket(8888)){
    
    for (int i = 0; i < INTRODUCER; i++) {
        string contact = "172.22.154." + std::to_string(182+i);
        known_hosts.push_back(contact);
        printf("%s is a known contact.\n", contact.c_str());
    }

    /* Flag == 1 means this is first machine in the cluster */
    if (flag == 1) {
        self_index = 1;
        string my_addr = "172.22.154.182";
        long long current_ts = unix_timestamp();
        VMNode tmp(my_addr, current_ts, self_index);
        member_list[self_index] = tmp;
        printf("%s\n", member_list[1].ip.c_str());
    } else {
        join();
    }
}

long long Daemon::unix_timestamp(){
    /*
    time_t t = std::time(0);
    long long now = static_cast<long long> (t);
    */
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int now = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return now;
}



void Daemon::receive() {
    while (true) {
        char buf[BUFSIZE];
        char rip[BUFSIZE];
        msg_socket.recv(rip, buf);
        /* [TODO] Transform to log */
        printf("Receive %s from %s\n", buf, rip);
        switch(buf[0]){
            case 'j': /* Join */
                joinHandler(rip);
                break;
            case 'l': /* Leave */
                break;
            case 'h': /* Heartbeat */
                break;
            case 'm': /* Membership */
                break;
            default:
                break;
        }
        
        /* 
            Process request in new thread
            process()
        
        */

        
    }
}


/*
string Daemon::getSelfAddress(){
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(ifa->ifa_name, "utun2") == 0) {
                return String(addressBuffer);
            }
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
    return "";
}
*/