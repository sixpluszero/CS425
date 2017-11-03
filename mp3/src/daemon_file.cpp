#include "daemon.hpp"

void Daemon::clientPut(TCPSocket *sock, string fname) {
    if (role != "Primary") return;
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
            }
        }
    } else {
        tcpSendString(sock, "ack");
    }
    string content = tcpRecvString(sock);
    plog("receive file %s(%d)", fname.c_str(), content.length());

    //saveFile(content, fname);
    /* This is super slow */
    /* [TODO] Refractor this area! */
    int cnt = 0;
    for (auto it = member_list.begin(); it != member_list.end(); it++) {

        /* Assuming no failure */
        TCPSocket sock_w(member_list[it->first].ip, BASEPORT+3);
        plog("test0: connected");        
        tcpSendString(&sock_w, "masterput;"+fname);
        string ack = tcpRecvString(&sock_w);
        plog("test1: %s", ack.c_str());
        tcpSendString(&sock_w, content);
        ack = tcpRecvString(&sock_w);
        plog("test2: %s", ack.c_str());

        string update = fname + "/" + std::to_string(it->first) + "/" + std::to_string(unixTimestamp());
        newFileMappingLocation(update);
        /* Send */
        for (auto it = master_list.begin(); it != master_list.end(); it++) {
            if (it->second == "Primary") continue;
            TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
            tcpSendString(&sock_, "newfloc;"+update);
        }
        cnt++;
        if (cnt == 3) {
            tcpSendString(sock, "finished");
        }
        if (cnt == 4) {
            break;
        }
    }
    plog("updated file mapping: %s", fileMappingToString().c_str());
}

void Daemon::saveFile(string content, string fname){
    string lfname = "./mp3/files/"+fname;
    FILE *fp = fopen(lfname.c_str(),"w");
    fprintf(fp, "%s", content.c_str());
    fclose(fp);
}

void Daemon::dataPut(TCPSocket *sock, string fname) {
    plog("in dataPut");
    tcpSendString(sock, "ack");
    plog("sent first ack");
    string content = tcpRecvString(sock);
    plog("receive replication file %s(%d)", fname.c_str(), content.length());
    saveFile(content, fname);
    tcpSendString(sock, "finished");
}