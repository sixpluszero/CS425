#include "daemon.hpp"

void Daemon::joinHandler(char *remote_ip) {
    //long long ts = unixTimestamp();
    int remote_pos = 0;
    
    remote_pos = newMember(remote_ip); /* update membership */

    /* Only handle initial coldstart state */
    if (member_list.size() <= 3 && master_list.size() < 3) {
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
            info = info + "," + master_list[remote_pos];
        }
        msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
       // printf("send update %s %s\n", member_list[it->first].ip.c_str(), info.c_str());
    }
    
    if (master_list.find(remote_pos) != master_list.end()) {
        TCPSocket sock(string(remote_ip), BASEPORT+3);
        tcpSendString(&sock, "newfmap;"+fileMappingToString());
        //plog("master recv: %s", tcpRecvString(&sock).c_str());
    }
    plog("insert update member list: %s", membersToString().c_str());
    plog("insert update contact list: %s", contactsToString().c_str());
    plog("insert update master list: %s", mastersToString().c_str());
}

void Daemon::updateHandler(string msg) {
    string backup_msg = msg;
    plog(msg);
    if (msg[7] == 'j') { /* A join message */
        msg = msg.substr(12, msg.length());
        // If the new node is a master..
        if (msg.find(',') != std::string::npos) {
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
            if (backup_msg.find("Primary") != std::string::npos) {
                master_list[tmp.id] = "Primary";
            }
            if (backup_msg.find("Backup") != std::string::npos) {
                master_list[tmp.id] = "Backup";
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
    } else if (msg[7] =='m') { /* A master assignment message */
        //plog("debug master assignment original message %s", msg.c_str());
        msg = msg.substr(14, msg.length());
        int idx = msg.find(",");
        int tmp = stoi(msg.substr(0, idx));
        string new_role = msg.substr(idx+1, msg.length());
        //plog("debug master assignment message %d %s", tmp, new_role.c_str());
        if ((master_list.find(tmp) != master_list.end()) && (master_list[tmp] == new_role)) { /* Have received this message before */
            return;
        } else {
            plog(backup_msg);
            master_list[tmp] = new_role;
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
            }
            plog("master update master list: %s", mastersToString().c_str());
            
            /* Master assignment about this node */
            if (idx == self_index) {
                if (isPrimary()) {
                    role = "Primary";
                } else {
                    role = "Backup";
                }
            }
        }
    } else if (msg[7] =='c') { /* A crash message */
        msg = msg.substr(13, msg.length());
        VMNode tmp(msg);
        if (member_list.find(tmp.id) == member_list.end()) {
            /* Have received this message before */
            return;
        } else {
            plog("hanlding: %s", backup_msg.c_str());
            member_list.erase(tmp.id);
            master_list.erase(tmp.id);
            long long ts = unixTimestamp();
            updateContact(ts);
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
            }
            plog("crash update member list: %s", membersToString().c_str());
            plog("crash update contact list: %s", contactsToString().c_str());
            plog("crash update master list: %s", mastersToString().c_str());

            if (isMaster()){
                clearNodeFile(tmp.id);
                plog("after failure clear: %s", fileMappingToString().c_str());
                if (isPrimary()){
                    clearNodeFile(tmp.id);
                    plog("after failure clear: %s", fileMappingToString().c_str());
                    if (member_list.size() >= 3 && master_list.size() < 3) {
                        assignBackup(3-master_list.size());
                    }
                    
                    std::thread fix_t(&Daemon::fixReplication, this);
                    fix_t.detach();
                    
                } else {
                    if (!hasPrimary() && isFirstBackup()) {
                        upgradeBackup();
                        std::thread fix_t(&Daemon::fixReplication, this);
                        fix_t.detach();
                    }
                }
            }
        }
    }  else { /* A leave message */
        msg = msg.substr(13, msg.length());
        VMNode tmp(msg);
        if (member_list.find(tmp.id) == member_list.end()) {
            /* Have received this message before */
            return;
        } else {
            plog("hanlding: %s", backup_msg.c_str());
            member_list.erase(tmp.id);
            master_list.erase(tmp.id);
            long long ts = unixTimestamp();
            updateContact(ts);
            /* Broadcast update */
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), backup_msg.c_str());
            }
            plog("leave update member list: %s", membersToString().c_str());
            plog("leave update contact list: %s", contactsToString().c_str());
            plog("leave update master list: %s", mastersToString().c_str());
            
            if (isMaster()){
                clearNodeFile(tmp.id);
                plog("after failure clear: %s", fileMappingToString().c_str());
                if (isPrimary()){
                    if (member_list.size() >= 3 && master_list.size() < 3) {
                        assignBackup(3-master_list.size());
                    }
                } else {
                    if (!hasPrimary() && isFirstBackup()) {
                        upgradeBackup();
                    }
                }
            }
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
