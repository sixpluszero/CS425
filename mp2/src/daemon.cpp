#include "daemon.hpp"
using namespace std;

Daemon::Daemon(int flag): msg_socket(UDPSocket(8888)){
    
    for (int i = 0; i < INTRODUCER; i++) {
        string contact = "172.22.154." + std::to_string(182+i);
        known_hosts.push_back(contact);
        printf("%s is a known contact.\n", contact.c_str());
    }

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
    time_t t = std::time(0);
    long long now = static_cast<long long> (t);
    return now;
}

void Daemon::join() {
    for (int i = 0; i < INTRODUCER; i++) {
        printf("%s\n", known_hosts[i].c_str());
        msg_socket.send(known_hosts[i].c_str(), "join");
        char buf[1000];
        char rip[100];
        msg_socket.recv(rip, buf);
        printf("%s\n", buf);
        break;
    }
}

void Daemon::receive() {
    while (true) {
        char buf[BUFSIZE];
        char rip[BUFSIZE];
        msg_socket.recv(rip, buf);
        //buf[recvBytes] = '\0';
        printf("Receive %s from %s\n", buf, rip);
        
        /* 
            Process request in new thread
            process()
        
        */

        string response = "";
        /* Marshel my membership */
        response = response + std::to_string(member_list.size());
        for (std::map<int, VMNode>::iterator it = member_list.begin(); it != member_list.end(); it++) {
            response += ",";
            response += (std::to_string((it->second).id) + "/" + std::to_string((it->second).join_timestamp) + "/" + (it->second).ip);
        }
        msg_socket.send(rip, response.c_str());
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