#include "daemon.hpp"


void Daemon::joinHandler(char *remote_ip) {
    //long long ts = unixTimestamp();    
    int remote_pos = 0;
    
    remote_pos = updateMember(remote_ip, 0); /* update membership */
    
    
    // Send update to neighbors;
    for (auto it = contact_list.begin(); it != contact_list.end(); it++) {
        string info = "update,join," + member_list[remote_pos].toString();
        msg_socket.send(member_list[it->first].ip.c_str(), info.c_str());
    }

    /* Send back message */
    string ret = "reply";
    ret += "," + std::to_string(remote_pos);
    ret += "," + std::to_string(member_list.size());
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
        ret += "," + (it->second).toString();
    }
    msg_socket.send(remote_ip, ret.c_str());
}
