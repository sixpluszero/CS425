#include "daemon.hpp"

void Daemon::savaInitPregelMaster() {
    string ack, cmd;
    SAVA_WORKER_CONN.clear();
    SAVA_NUM_VERTICES = SAVA_VERTEX_MAPPING.size();
    cmd = "savaclientinit;" + to_string(SAVA_NUM_VERTICES);
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        TCPSocket *sock_w = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
        sock_w->sendStr(cmd);
        SAVA_WORKER_CONN[i] = sock_w;
    }

    for (auto x : SAVA_WORKER_CONN) {
        x.second->recvStr(ack);
        plog("Recv init ack from worker %d", x.first);
    }
}

void Daemon::savaMasterSuperstepThread(int wid){
    //plog("Entering worker %d(%s %s) thread function", wid, SAVA_APP_NAME.c_str(), SAVA_COMBINATOR.c_str());
    if (SAVA_WORKER_CONN[wid]->sendStr("savaclientstep;"+std::to_string(SAVA_ROUND)) == 0) {
        //plog("Send step command to %d", wid);
    } else {
        plog("Error in superstep communication %d", wid);
        return;
    }
    if (SAVA_WORKER_CONN[wid]->recvFile("./mp4/tmp/rmsg_in_" + std::to_string(wid) + ".txt") == 0) {
        plog("Recv msg file from %d", wid);
    } else {
        plog("Error in superstep msg file %d", wid);
        return;
    }
}

int  Daemon::savaMasterSuperstep() {
    /**
     * (1) Send savaclientstep with step number parameter to worker.
     * (2) Wait for execution finished notification
     * (3) Request for outgoing msgs
     * (4) Distributed msgs to target worker
     * (5) Receive active node number from each worker
     * (6) Decide whether to stop
     * (7) Gather result
     */

    FILE *fp;
    string fname, msg, ack;
    int id, num, count;
    double value;
    vector<std::thread> ths;

    SAVA_WORKER_CONN.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        TCPSocket *sock_w = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
        SAVA_WORKER_CONN[i] = sock_w;
        ths.push_back(std::thread(&Daemon::savaMasterSuperstepThread, this, i));
    }
    for (auto& th : ths) th.join();
    plog("Receive messages from each worker");
    
    auto start = std::chrono::system_clock::now();
    SAVA_REMOTE_MSGS.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        int cnt = 0;
        fname = "./mp4/tmp/rmsg_in_" + std::to_string(i) + ".txt";
        fp = fopen(fname.c_str(), "r");
        if (fp == NULL) continue;
        while (fscanf(fp, "%d %d", &id, &num) != EOF) {
            cnt++;
            if (SAVA_REMOTE_MSGS.find(id) == SAVA_REMOTE_MSGS.end()) {
                vector<double> tmp;
                SAVA_REMOTE_MSGS[id] = tmp;
            }
            for (int w = 0; w < num; w++) {
                fscanf(fp, "%lf", &value);
                SAVA_REMOTE_MSGS[id].push_back(value);
            }
        }
        plog("worker %d send %d lines messages", i, cnt);
        fclose(fp);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> dif = end - start;
    plog("Remote mssages receiving using %lf seconds", dif.count());
    plog("Receive messages files from each worker");
    

    start = std::chrono::system_clock::now();
    FILE *fps[SAVA_NUM_WORKER];
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fname = "./mp4/tmp/rmsg_out_" + std::to_string(i) + ".txt";
        fps[i-1] = fopen(fname.c_str(), "w");
    }
    for (auto x : SAVA_REMOTE_MSGS) {
        int idx = SAVA_VERTEX_MAPPING[x.first] - 1;
        fprintf(fps[idx], "%d %lu", x.first, x.second.size());
        for (size_t w = 0; w < x.second.size(); w++) {
            fprintf(fps[idx], " %lf", x.second[w]);
        }
        fprintf(fps[idx], "\n");
    }
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fclose(fps[i-1]);
    }
    end = std::chrono::system_clock::now();
    dif = end - start;
    plog("Remote mssages processing using %lf seconds", dif.count());

    plog("Generate messages files for each worker");
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fname = "./mp4/tmp/rmsg_out_" + std::to_string(i) + ".txt";
        SAVA_WORKER_CONN[i]->sendFile(fname.c_str());
    }
    plog("Send messages files to each worker");
    
    count = 0;
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        SAVA_WORKER_CONN[i]->recvStr(ack);
        plog("Recv %s active nodes from %d", ack.c_str(), i);
        count += stoi(ack);
    }
    plog("Active nodes after this round is %d", count);
    if (count == 0) SAVA_STATE = 2;
    return 0;
}

