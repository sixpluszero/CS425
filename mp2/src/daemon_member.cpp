#include "daemon.hpp"
using namespace std;

void Daemon::updateContact(long long ts){
    vector<int> idx;
    int tmp = 0, my_loc = 0, left_cnt = 0, right_cnt = 0;
    /*
    map<int, long long> tmp_list;
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        tmp_list[it->first] = it->second;
    }
    */
    contact_list.clear();

    for (auto it = member_list.begin(); it != member_list.end(); it++){
        idx.push_back(it->first);
    }
    sort(idx.begin(), idx.end());
    for (auto i = 0; i < idx.size(); i++){
        if (idx[i] == self_index) {
            my_loc = i;
            break;
        }
    }
    tmp = my_loc + 1;
    while (right_cnt < 2 && tmp != my_loc) {
        if (tmp == int(idx.size())) tmp = 0;
        if (tmp == my_loc) break;
        contact_list[idx[tmp]] = ts;
        right_cnt++;
        tmp++;
    }
    tmp = my_loc - 1;
    while (left_cnt < 2 && tmp != my_loc) {
        if (tmp == -1) tmp = int(idx.size())-1;
        if (tmp == my_loc) break;
        contact_list[idx[tmp]] = ts;
        left_cnt++;
        tmp--;
    }
    /*
    for (auto it = tmp_list.begin(); it != tmp_list.end(); it++) {
        if (contact_list.find(it->first) != contact_list.end()){
            contact_list[it->first] = it->second;
        }
    }
    */

    plog("update contact list: %s", contactsToString().c_str());
}

int Daemon::newMember(char *remote_ip) {
    long long ts = unixTimestamp();
    int remote_pos = 0;
    for (int i = 1; i <= NODE; i++) {
        if (member_list.find(i) == member_list.end()){
            string rip = remote_ip;
            VMNode tmp(rip, ts, i);
            member_list[i] = tmp;
            remote_pos = i;
            plog("insert %s in %dth position", remote_ip, remote_pos);
            break;
        }
    }
    updateContact(ts); /* update contact */
    return remote_pos;
}


void Daemon::setMemberList(string w) {
    int pIdx = w.find(",");
    w = w.substr(pIdx+1, w.length());
    while (w.length()) {
        pIdx = w.find(",");
        if (pIdx == -1) pIdx = w.length();
        VMNode newNode(w.substr(0, pIdx));
        member_list[newNode.id] = newNode;
        if (pIdx < w.length()) {
            w = w.substr(pIdx+1, w.length());
        } else {
            break;
        }
    }
    plog("init member: %s", membersToString().c_str());
}

string Daemon::membersToString(){
    string ans = "";
    ans += std::to_string(member_list.size());
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
        ans += "," + (it->second).toString();
    }
    return ans;
}

string Daemon::contactsToString(){
    string ans = "";
    ans += std::to_string(contact_list.size());
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        ans += "," + std::to_string(it->first) + "/" + std::to_string(it->second);
    }
    return ans;
}
