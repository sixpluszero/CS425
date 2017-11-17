#include "daemon.hpp"
using namespace std;

/**
 *  Below is functions related to master relationship 
 */

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
            sock.sendStr("newfamp;"+fileMappingToString());
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

/**
 *  Below is functions related to file_location data structure
 */

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

/* Rest full file_location data structure with given input */
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

/* Add single file replica location to a given file */
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
