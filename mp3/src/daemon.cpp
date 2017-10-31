#include "daemon.hpp"
using namespace std;

Daemon::Daemon(int flag): msg_socket(UDPSocket((BASEPORT+1))), cmd_socket(UDPSocket((BASEPORT+2))), out_socket(UDPSocket((BASEPORT), true)), file_socket(UDPSocket((BASEPORT+3))){
    member_list.clear();
    contact_list.clear();

    srand(time(NULL));
    setSelfAddr();
    setLogFile();

    for (int i = 0; i < INTRODUCER; i++) {
        string contact = "172.22.154." + std::to_string(182+i);
        known_hosts.push_back(contact);
        plog("%s is a known introducer", contact.c_str());
    }

    join();

}

void Daemon::join() {
    int idx;
    bool join = false;
    while (!join) {
        for (int i = 0; i < INTRODUCER; i++) {
            if (strcmp(known_hosts[i].c_str(), self_ip.c_str()) == 0) continue;
            msg_socket.send(known_hosts[i].c_str(), "join");
    
            char buf[BUFSIZE], rip[100];
            int recvBytes = 0;
            msg_socket.setTimeout(500000);
            recvBytes = msg_socket.recv(rip, buf);
            //plog("introducer %s return %d bytes", known_hosts[i].c_str(), recvBytes);
            if (recvBytes <= 0) {
                plog("introducer %s time out for join request", known_hosts[i].c_str());
                continue;   
            }
            msg_socket.setTimeout(0);
            string w = buf;
            if (w[0] != 'r') continue;
            idx = w.find(";");
            w = w.substr(idx + 1, w.length());
            idx = w.find(",");
            self_index = stoi(w.substr(0, idx));
            idx = w.find(";");
            setMemberList(w.substr(0, idx));
            long long ts = unixTimestamp();
            updateContact(ts);
            w = w.substr(idx + 1, w.length());
            setMasterList(w);
            join = true;
            break;
        }
        if (join == true) break;
        /* We are the live node in cluster */
        plog("first node in cluster");
        self_index = 1;
        long long current_ts = unixTimestamp();
        VMNode tmp(self_ip, current_ts, self_index);
        member_list[self_index] = tmp;
        master_list.clear();
        master_list[1] = "Primary";
    }
}

void Daemon::command() {
    plog("monitoring command line message");
    while (leave_flag == false) {
        char buf[BUFSIZE];
        char rip[BUFSIZE];
        cmd_socket.recv(rip, buf);
        plog("[cmd debug] %s", buf);
        string msg;
        switch(buf[0]){
            case 'l':
                plog("[cmd debug] leave should happen");
                //msg = "exit";
                //msg_socket.send(member_list[self_index].ip.c_str(), msg.c_str());
                for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                    string info = "update,leave," + member_list[self_index].toString();
                    msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
                }
                leave_flag = true;
                break;
            case 'i':
                msg = member_list[self_index].toString();
                out_socket.send(rip, msg.c_str());
                break;
            case 'm':
                msg = membersToString();
                out_socket.send(rip, msg.c_str());
                break;                
            default:
                break;
        }
    }
}

void Daemon::heartbeat() {
    while (leave_flag == false) {
        usleep(HEARTBEAT);
        for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
            string info = "heartbeat";
            if (!dropMsg()) msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
        }
    }
    plog("module heartbeat exit");
}

void Daemon::timeout() {
    while (leave_flag == false) {
        usleep(SCAN);
        /* [TODO] Add mutex lock to this */
        long long ts = unixTimestamp();
        vector<int> to_remove;
        vector<string> del_node;
        for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
            if ((ts - it->second) > FAILURE) {
                plog("crash %d(%s/%lld) failed (latest %lld)", it->first, member_list[it->first].ip.c_str(), member_list[it->first].join_timestamp, it->second);
                to_remove.push_back(it->first);
                
            }
        }
        for (auto it = to_remove.begin(); it != to_remove.end(); it++) {
            int pos = *it;
            del_node.push_back(member_list[pos].toString());
            member_list.erase(pos);
            updateContact(ts);
        }

        for (auto it_ = del_node.begin(); it_ != del_node.end(); it_++) {
            string info = "update,crash," + (*it_);
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
            }                
        }

        if (to_remove.size() > 0) {
            plog("crash update member list: %s", membersToString().c_str());
            plog("crash update contact list: %s", contactsToString().c_str());
        }
    }
    plog("module timeout exit");
}

void Daemon::receive() {
    while (leave_flag == false) {
        char buf[BUFSIZE];
        char rip[BUFSIZE];
        msg_socket.recv(rip, buf);
        
        /* 
            [TODO] Process request in new thread
            process()
        */

        switch(buf[0]){
            case 'j': /* Join */
                joinHandler(rip);
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
    plog("module receive exit");
}
