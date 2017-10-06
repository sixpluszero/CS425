#include "daemon.hpp"


void Daemon::joinHandler(char *remote_ip) {
    //long long ts = unixTimestamp();    
    int remote_pos = 0;
    
    remote_pos = newMember(remote_ip); /* update membership */
    log("update member %s", membersToString().c_str());
    log("update contact %s", contactsToString().c_str());
    /* Send back message */
    string ret = "reply";
    ret += "," + std::to_string(remote_pos);
    ret += "," + std::to_string(member_list.size());
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
        ret += "," + (it->second).toString();
    }
    msg_socket.send(remote_ip, ret.c_str());


    // Send update to neighbors;
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        string info = "update,join," + member_list[remote_pos].toString();
        msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
        printf("send update %s %s\n", member_list[it->first].ip.c_str(), info.c_str());
    }

}

void Daemon::updateHandler(string msg) {
    log(msg);
    if (msg[7] == 'j') { /* A join message */
        msg = msg.substr(12, msg.length());
        VMNode tmp(msg);
        if (member_list.find(tmp.id) != member_list.end()) {
            /* There is a same node taking this position */
            return;
        } else {
            member_list[tmp.id] = tmp;
            long long ts = unixTimestamp();
            updateContact(ts);
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), msg.c_str());
            }
        }
    } else { /* A leave message */

    }
}

void Daemon::heartbeatHandler(char *remote_ip) {
    long long ts = unixTimestamp();
    string rip = remote_ip;
    
    /* [TODO] Need to add lock */
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        if (member_list[it->first].ip == rip) {
            contact_list[it->first] = ts;
            log("heartbeat from %s", remote_ip);
        }
    }
            
}
