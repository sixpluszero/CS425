#include "daemon.hpp"


void Daemon::joinHandler(char *remote_ip) {
    //long long ts = unixTimestamp();    
    int remote_pos = 0;
    
    remote_pos = newMember(remote_ip); /* update membership */
    plog("join update member list: %s", membersToString().c_str());
    plog("join update contact list: %s", contactsToString().c_str());

    // [TODO] Master
    if (master_list.size() <= 3) {
        master_list[remote_pos] = "Backup";
    }

    /* Send back message */
    string ret = "reply";
    ret += ";" + std::to_string(remote_pos);
    ret += ";" + membersToString();
    ret += ";" + mastersToString();
    msg_socket.send(remote_ip, ret.c_str());


    // Send update to neighbors;
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        string info = "update,join," + member_list[remote_pos].toString();
        if (master_list.find(remote_pos) != master_list.end()) {
            info = info + "," + std::to_string(remote_pos);
        }
        msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
       // printf("send update %s %s\n", member_list[it->first].ip.c_str(), info.c_str());
    }

}

void Daemon::updateHandler(string msg) {
    string backup_msg = msg;
    //plog(msg);
    if (msg[7] == 'j') { /* A join message */
        msg = msg.substr(12, msg.length());
        // If the new node is a master..
        if (msg.find(',') != -1) {
            msg = msg.substr(0, msg.find(','));
        }
        VMNode tmp(msg);
        if (member_list.find(tmp.id) != member_list.end()) {
            /* There is a same node taking this position */
            //plog("%s has been stored", tmp.toString().c_str());
            return;
        } else {
            plog(backup_msg);
            member_list[tmp.id] = tmp;
            long long ts = unixTimestamp();
            updateContact(ts);
            if (backup_msg.find('Primary') != -1) {
                master_list[tmp.id] = 'Primary';
            }
            if (backup_msg.find('Backup') != -1) {
                master_list[tmp.id] = 'Backup';
            }

            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
                //plog("debug: sending \'%s\' to %d", backup_msg.c_str(), it->first);
            }
            plog("join update member list: %s", membersToString().c_str());
            plog("join update contact list: %s", contactsToString().c_str());
            plog("join update master list: %s", mastersToString().c_str());
        }
    } else if (msg[7] =='c') { /* A crash message */
        msg = msg.substr(13, msg.length());
        VMNode tmp(msg);
        if (member_list.find(tmp.id) == member_list.end()) {
            /* Have received this message before */
            return;
        } else {
            plog(backup_msg);
            member_list.erase(tmp.id);
            long long ts = unixTimestamp();
            updateContact(ts);
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
            }
            plog("crash update member list: %s", membersToString().c_str());
            plog("crash update contact list: %s", contactsToString().c_str());
        }
    } else { /* A leave message */
        msg = msg.substr(13, msg.length());
        VMNode tmp(msg);
        if (member_list.find(tmp.id) == member_list.end()) {
            /* Have received this message before */
            return;
        } else {
            plog(backup_msg);
            member_list.erase(tmp.id);
            long long ts = unixTimestamp();
            updateContact(ts);
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
            }
            plog("leave update member list: %s", membersToString().c_str());
            plog("leave update contact list: %s", contactsToString().c_str());
        }
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
