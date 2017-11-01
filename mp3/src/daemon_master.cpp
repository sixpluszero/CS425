#include "daemon.hpp"
using namespace std;

bool Daemon::isPrimary(){
    if (master_list.find(self_index) == master_list.end()) return false;
    return master_list[self_index] == "Primary";
}

bool Daemon::isBackup(){
    if (master_list.find(self_index) == master_list.end()) return false;
    return master_list[self_index] == "Backup";
}

bool Daemon::isMaster(){
    return (master_list.find(self_index) != master_list.end());
}

bool Daemon::hasPrimary(){
    for (auto it = master_list.begin(); it != master_list.end(); it++) {
        if (it->second == "Primary") return true;
    }
    return false;
}

bool Daemon::isFirstBackup(){
    if (role != "Backup") return false;
    for (auto it = master_list.begin(); it != master_list.end(); it++) {
        if (it->second == "Backup" && it->first < self_index) return false;
    }
    return true;
}

void Daemon::assignBackup(int num) {
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
        if (master_list.find(it->first) == master_list.end()) {
            master_list[it->first] = "Backup";
            string msg = "update,master," + std::to_string(it->first)+",Backup";
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), msg.c_str());
            }
        }
    }
}

void Daemon::upgradeBackup() {
    role = "Primary";
    master_list[self_index] = "Primary";
    string msg = "update,master," + std::to_string(self_index)+",Primary";
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        msg_socket.send(member_list[it->first].ip.c_str(), msg.c_str());
    }
    // Notify
    assignBackup(3 - master_list.size());
}