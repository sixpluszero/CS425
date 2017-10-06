#include "daemon.hpp"
using namespace std;

Daemon::Daemon(int flag): msg_socket(UDPSocket(8888)){
    
    for (int i = 0; i < INTRODUCER; i++) {
        string contact = "172.22.154." + std::to_string(182+i);
        known_hosts.push_back(contact);
        printf("%s is a known contact.\n", contact.c_str());
    }
    member_list.clear();
    contact_list.clear();

    /* Flag == 1 means this is first machine in the cluster */
    if (flag == 1) {
        self_index = 1;
        string my_addr = "172.22.154.182";
        long long current_ts = unixTimestamp();
        VMNode tmp(my_addr, current_ts, self_index);
        member_list[self_index] = tmp;
    } else {
        join();
    }
}

void Daemon::join() {
    int idx;
    for (int i = 0; i < INTRODUCER; i++) {
        //printf("%s\n", known_hosts[i].c_str());
        msg_socket.send(known_hosts[i].c_str(), "join");
        while (true) {
            char buf[1000];
            char rip[100];
            msg_socket.recv(rip, buf);
            string w = buf;
            if (w[0] != 'r') continue;
            idx = w.find(",");
            w = w.substr(idx + 1, w.length());
            idx = w.find(",");
            self_index = stoi(w.substr(0, idx));
            setMemberList(w);
            long long ts = unixTimestamp();
            updateContact(ts);
            //log(contactsToString());
            //printf("%lld recv join msg: %s\n", unixTimestamp(), buf);
            break;    
        }
        break;
    }   
}

void Daemon::receive() {
    while (true) {
        char buf[BUFSIZE];
        char rip[BUFSIZE];
        msg_socket.recv(rip, buf);
        
        /* 
            [TODO] Process request in new thread
            process()
        
        */

        /* [TODO] Transform printf to log */
        //printf("Receive %s from %s\n", buf, rip);
        switch(buf[0]){
            case 'j': /* Join */
                joinHandler(rip);
                break;
            case 'l': /* Leave */
                break;
            case 'h': /* Heartbeat */
                heartbeatHandler(rip);
                break;
            case 'u': /* Membership */
                updateHandler(string(buf));
                break;
            default:
                break;
        }
    }
}
