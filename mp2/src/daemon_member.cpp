#include "daemon.hpp"
using namespace std;

void Daemon::updateContact(long long ts){
    vector<int> idx;
    int tmp = 0, my_loc = 0, left_cnt = 0, right_cnt = 0;
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
        if (tmp == 0) tmp = int(idx.size())-1;
        if (tmp == my_loc) break;
        contact_list[idx[tmp]] = ts;
        left_cnt++;
        tmp--;
    }
}

int Daemon::updateMember(char *remote_ip, int flag) {
    long long ts = unixTimestamp();
    int remote_pos = 0;
    if (flag == 0) { /* Join member */
        for (int i = 1; i <= NODE; i++) {
            if (member_list.find(i) == member_list.end()){
                string rip = remote_ip;
                VMNode tmp(rip, ts, i);
                member_list[i] = tmp;
                remote_pos = i;
                printf("[LOG] %s join at %lld in ring %d\n", remote_ip, ts, remote_pos);
                break;
            }
        }
        return remote_pos;
    } else { /* Leave(Crash) member */
        // [TODO]
    }
    updateContact(ts); /* update contact */
    return remote_pos;
}


void Daemon::setMemberList(string w) {
    int pIdx = w.find(",");
    //int n = stoi(w.substr(0, pIdx));
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
}

