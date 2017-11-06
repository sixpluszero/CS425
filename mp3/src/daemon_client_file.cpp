#include "daemon.hpp"

/* Master: Client put */
void Daemon::clientPut(TCPSocket *sock, string fname) {
    if (role != "Primary"){
        tcpSendString(sock, "rej");
        return;
    }
    if (hasFile(fname)) { /* We are going to do an update */
        long long ots = fileLatestTime(fname);
        long long nts = unixTimestamp();
        if ((nts - ots) < 60 * 1000) {
            tcpSendString(sock, "has old replica in last minute");
            string cmd = tcpRecvString(sock);
            if (cmd == "No"){
                plog("User reject file update");
                return;
            } else {
                plog("User accept file update");
                tcpSendString(sock, "ack");
            }
        } else {
            tcpSendString(sock, "ack");
        }
    } else {
        tcpSendString(sock, "ack");
    }
    string tmp_file = "./mp3/tmp/"+fname;
    recvFile(sock, tmp_file);
    plog("file received at temp file %s", tmp_file.c_str());

    vector< int > cand;
    if (hasFile(fname)) {
        for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
            cand.push_back(it->first);
        }
    } else {
        for (auto it = member_list.begin(); it != member_list.end(); it++) {
            cand.push_back(it->first);
        }    
    }
    std::random_shuffle(cand.begin(), cand.end());

    int cnt = 0;
    for (auto it = cand.begin(); it != cand.end(); it++) {
        int nid = *it; 
        string ack;
        /* Assuming no failure */
        TCPSocket sock_w(member_list[nid].ip, BASEPORT+3);
        tcpSendString(&sock_w, "fileput;"+fname);
        ack = tcpRecvString(&sock_w);
        if (ack == "error") {
            plog("Error in replicating new file to %d", nid);
            continue;
        }
        sendFile(&sock_w, tmp_file);
        ack = tcpRecvString(&sock_w);
        plog("response from replica server: %s", ack.c_str());
        if (ack == "success") {
            string update = fname + "/" + std::to_string(nid) + "/" + std::to_string(unixTimestamp());
            newFileMappingLocation(update);
            /* Send */
            for (auto it = master_list.begin(); it != master_list.end(); it++) {
                if (it->second == "Primary") continue;
                TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
                tcpSendString(&sock_, "newfloc;"+update);
            }
            cnt++;
            if (cnt == 3) {
                plog("reach quorum");
                tcpSendString(sock, "success");
            }
            if (cnt == 4) {
                break;
            }    
        } else {
            plog("Error in replicating new file to %d", nid);
            continue;
        }
    }
    plog("updated file mapping: %s", fileMappingToString().c_str());
}

void Daemon::clientGet(TCPSocket *sock, string fname){
    string ack;
    if (role != "Primary"){
        tcpSendString(sock, "rej");
        return;
    }
    if (hasFile(fname)) { 
        tcpSendString(sock, "ack");
        ack = tcpRecvString(sock);
        for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
            TCPSocket sock_w(member_list[it->first].ip, BASEPORT+3);
            tcpSendString(&sock_w, "fileget;"+fname);
            recvFile(&sock_w, "./mp3/tmp/"+fname);
            sendFile(sock, "./mp3/tmp/"+fname);
            break;
        }
        return;
    } else {
        tcpSendString(sock, "file not exists");
        return;
    }

}

void Daemon::clientDel(TCPSocket *sock, string fname){
    string ack;
    if (role != "Primary"){
        tcpSendString(sock, "rej");
        return;
    }
    if (hasFile(fname)) {
        for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
            TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
            tcpSendString(&sock_, "filedel;"+fname);
        }
        file_location.erase(fname);
        for (auto it = master_list.begin(); it != master_list.end(); it++) {
            if (it->second == "Primary") continue;
            TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
            tcpSendString(&sock_, "masterfiledel;"+fname);
        }
        tcpSendString(sock, "success");
        return;
    } else {
        tcpSendString(sock, "file not exists");
        return;        
    }
}

void Daemon::clientList(TCPSocket *sock, string fname){
    string ack;
    if (role != "Primary"){
        tcpSendString(sock, "rej");
        return;
    }
    if (hasFile(fname)) {
        string result;
        for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
            if (result != "") result += ",";
            result += member_list[it->first].ip;
        }
        result = "(" + result + ")";
        tcpSendString(sock, result);
        return;
    } else {
        tcpSendString(sock, "file not exists");
        return;        
    }
}
