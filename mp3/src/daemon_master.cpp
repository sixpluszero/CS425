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
    int cnt = num;
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
        if (master_list.find(it->first) == master_list.end()) {
            master_list[it->first] = "Backup";
            TCPSocket sock(member_list[it->first].ip, BASEPORT+3);
            tcpSendString(&sock, "newfamp;"+fileMappingToString());
            string msg = "update,master," + std::to_string(it->first)+",Backup";
            for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
                msg_socket.send(member_list[it->first].ip.c_str(), msg.c_str());
            }
            cnt--;
            if (cnt == 0) break;
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
    assignBackup(3 - master_list.size());
}

void Daemon::initFileMapping() {
    file_location.clear();
}

void Daemon::clearNodeFile(int id) {
    for (auto it = file_location.begin(); it != file_location.end(); it++) {
        it->second.erase(id);
    }
}

string Daemon::fileMappingToString() { /* format: fname:id/ts,id/ts; */
    string ret = "";
    for (auto it = file_location.begin(); it != file_location.end(); it++) {
        string sub = "";
        for (auto gt = it->second.begin(); gt != it->second.end(); gt++) {
            if (sub != "") sub = sub + ",";
            sub = sub + std::to_string(gt->first) + "/" + std::to_string(gt->second);
        }
        sub = it->first + ":" + sub;
        if (ret != "") ret = ret + ";";
        ret = ret + sub;
    }
    //plog("marshal file mapping is: %s", ret.c_str());
    return ret;
}

void Daemon::newFileMapping(string input) { /* format: fname:id/ts,id/ts; */
    file_location.clear();
    while (input != "") {
        int idx = input.find(";");
        if (idx == -1) idx = input.length();
        string file = input.substr(0, idx);
        int idx2 = file.find(":");
        string fname = file.substr(0, idx2);
        file = file.substr(idx2+1, file.length());
        map<int, long long> tmp_map;
        while (file != "") {
            int idx3 = file.find(",");
            if (idx3 == -1) idx3 = file.length();
            string tmp = file.substr(0, idx3);
            tmp_map[stoi(tmp.substr(0, tmp.find("/")))] = stoll( tmp.substr(tmp.find("/")+1, tmp.length()));
            if (idx3 == int(file.length())) break;
            file = file.substr(idx3+1, file.length());    
        }
        file_location[fname] = tmp_map;
        if (idx == int(input.length())) break;
        input = input.substr(idx+1, input.length());
    }
}

void Daemon::newFileMappingLocation(string input) { /* format: fname/id/ts */
    int idx;
    idx = input.find("/");
    string fname = input.substr(0, idx);
    input = input.substr(idx+1, input.length());
    idx = input.find("/");
    int nid = stoi(input.substr(0, idx));
    input = input.substr(idx+1, input.length());
    long long ts = stoll(input);
    if (file_location.find(fname) == file_location.end()) {
        map<int, long long> tmp_map;
        tmp_map[nid] = ts;
        file_location[fname] = tmp_map;
    } else {
        file_location[fname][nid] = ts;
    }
}

bool Daemon::hasFile(string fname) {
    return (file_location.find(fname) != file_location.end());
}

int Daemon::replicaCount(string fname) {
    if (!hasFile(fname)) return 0;
    return int(file_location[fname].size());
}

long long Daemon::fileLatestTime(string fname) {
    if (!hasFile(fname)) return 0;
    long long ret;
    for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
        ret = max(ret, it->second);
    }
    return ret;
}

/* Master: Issue re-replication */
void Daemon::fixReplication(){
    plog("start fixing replication");
    for (auto it = file_location.begin(); it != file_location.end(); it++) {
        if (it->second.size() == 4) continue;
        int src_node = it->second.begin()->first;
        string fname = it->first;
        plog("fixing %s using replica at %d", fname.c_str(), src_node);
        while (it->second.size() < 4 && member_list.size() >= 4) {
            for (auto it_0 = member_list.begin(); it_0 != member_list.end(); it_0++) {
                if (it->second.find(it_0->first) == it->second.end()) {
                    // This is a data slot to fit in
                    int dst_node = it_0->first;
                    plog("fixing %s picking %d", fname.c_str(), dst_node);
                    TCPSocket sock_(member_list[src_node].ip, BASEPORT+3);
                    tcpSendString(&sock_, "copy;"+std::to_string(dst_node)+"/"+fname);
                    string ack = tcpRecvString(&sock_);
                    plog("node %d reply with %s", src_node, ack.c_str());
                    if (ack == "success"){
                        plog("add replication of %s from %d to %d", fname.c_str(), src_node, dst_node);
                        string update = fname + "/" + std::to_string(dst_node) + "/" + std::to_string(unixTimestamp());
                        newFileMappingLocation(update);
                        for (auto it2 = master_list.begin(); it2 != master_list.end(); it2++) {
                            if (it2->second == "Primary") continue;
                            TCPSocket sock_m(member_list[it2->first].ip, BASEPORT+3);
                            tcpSendString(&sock_m, "newfloc;"+update);
                        }
                        break;
                    }
                }
            }
        }
    }   
    plog(fileMappingToString().c_str());
    plog("end fixing replication");
}