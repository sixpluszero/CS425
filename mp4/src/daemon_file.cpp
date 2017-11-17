#include "daemon.hpp"

int Daemon::putFile(TCPSocket *sock, string fname) {
    int r;
    r = sock->sendStr("ack");
    if (r == 1) return r;
    r = sock->recvFile(fname);
    if (r == 1) return r;
    r = sock->sendStr("success");
    if (r == 1) return r;
    return 0;
}

/* Response to the data saving request */
void Daemon::replicateFile(TCPSocket *sock, string input) {
    int r;
    int dst_node = stoi(input.substr(0, input.find("/")));
    string fname = input.substr(input.find("/")+1, input.length());
    TCPSocket sock_w(member_list[dst_node].ip, BASEPORT+3);
    string ack;
    r = sock_w.sendStr("fileput;"+fname);
    if (r == 1) {
        sock->sendStr("fail");
        return;
    }
    r = sock_w.recvStr(ack);
    if (r == 1) {
        sock->sendStr("fail");
        return;
    }   
    r = sock_w.sendFile("./mp4/files/"+fname);
    if (r == 1) {
        sock->sendStr("fail");
        return;
    }    
    r = sock_w.recvStr(ack);
    if (r == 0){
        sock->sendStr("success");
    } else {
        sock->sendStr("fail");
    }
    
}


/**
 *  fixReplication() starts re-replication issued by master 
 */
void Daemon::fixReplication(){
    plog("start fixing replication");
    for (auto it = file_location.begin(); it != file_location.end(); it++) {
        if (it->second.size() == 4) continue;
        while (it->second.size() < 4 && member_list.size() >= 4) { /* If data node is less than four, don't fix. */
            /* Randomly pick one node that has this file */
            vector<int> cand;
            for (auto it_0 = it->second.begin(); it_0 != it->second.end(); it_0++) {
                cand.push_back(it_0->first);
            }
            std::random_shuffle(cand.begin(), cand.end());
            int src_node = cand[0];
            plog("replica src node is %d", src_node);
            string fname = it->first;
            plog("fixing %s using replica at %d", fname.c_str(), src_node);
            while (it->second.size() < 4 && member_list.size() >= 4) {
                /* Randomly pick a new data node */
                cand.clear();
                for (auto it_0 = member_list.begin(); it_0 != member_list.end(); it_0++) {
                    cand.push_back(it_0->first);
                }
                std::random_shuffle(cand.begin(), cand.end());
                for (auto it_0 = cand.begin(); it_0 != cand.end(); it_0++) {
                    if (it->second.find(*it_0) == it->second.end()) {
                        // This is a data slot to fit in
                        int dst_node = *it_0;
                        plog("replica dst node is %d", dst_node);
                        plog("fixing %s picking %d", fname.c_str(), dst_node);
                        try {
                            TCPSocket sock_(member_list[src_node].ip, BASEPORT+3);
                            string ack;
                            int r;
                            r = sock_.sendStr("copy;"+std::to_string(dst_node)+"/"+fname);
                            if (r == 1) {
                                continue;
                            }
                            r = sock_.recvStr(ack);
                            if (r == 1) {
                                continue;
                            }
                            plog("node %d reply with %s", src_node, ack.c_str());
                            if (ack == "success"){
                                plog("add replication of %s from %d to %d", fname.c_str(), src_node, dst_node);
                                string update = fname + "/" + std::to_string(dst_node) + "/" + std::to_string(unixTimestamp());
                                newFileMappingLocation(update);
                                for (auto it2 = master_list.begin(); it2 != master_list.end(); it2++) {
                                    if (it2->second == "Primary") continue;
                                    TCPSocket sock_m(member_list[it2->first].ip, BASEPORT+3);
                                    int r = sock_m.sendStr("newfloc;"+update);
                                    if (r == 1) {
                                        continue;
                                    }
                                }
                                break;
                            } else {
                                plog("Error in replicating(dst).");    
                                continue;
                            }
                        } catch (...) {
                            plog("Error in replicating(src).");
                            continue;
                        }
                    }
                }
            }
        }
    }   
    plog(fileMappingToString().c_str());
    plog("end fixing replication");
}