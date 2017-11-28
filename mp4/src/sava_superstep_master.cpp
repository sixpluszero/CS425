#include "daemon.hpp"

void Daemon::savaInitPregelMaster() {
    string ack;

    SAVA_WORKER_CONN.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        TCPSocket *sock_w = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
        sock_w->sendStr("savaclientinit");
        SAVA_WORKER_CONN[i] = sock_w;
    }

    for (auto x : SAVA_WORKER_CONN) {
        x.second->recvStr(ack);
        plog("Recv init ack from worker %d", x.first);
    }
}

void Daemon::savaMasterSuperstepThread(int wid){
    plog("Entering worker %d(%s %s) thread function", wid, SAVA_APP_NAME.c_str(), SAVA_COMBINATOR.c_str());
    if (SAVA_WORKER_CONN[wid]->sendStr("savaclientstep;"+std::to_string(SAVA_ROUND)) == 0) {
        plog("Send step command to %d", wid);
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
     */

    FILE *fp;
    string fname, msg, ack;
    int id, num, count;
    double value;

    vector<std::thread> ths;
    /* Step 1 */
    SAVA_WORKER_CONN.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        TCPSocket *sock_w = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
        //sock_w->sendStr("savaclientinit");
        SAVA_WORKER_CONN[i] = sock_w;        
        ths.push_back(std::thread(&Daemon::savaMasterSuperstepThread, this, i));
    }
    for (auto& th : ths) th.join();
    plog("Receive messages from each worker");

    SAVA_REMOTE_MSGS.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {

        fname = "./mp4/tmp/rmsg_in_" + std::to_string(i) + ".txt";
        fp = fopen(fname.c_str(), "r");
        if (fp == NULL) continue;
        while (fscanf(fp, "%d %d", &id, &num) != EOF) {
            if (SAVA_REMOTE_MSGS.find(id) == SAVA_REMOTE_MSGS.end()) {
                vector<double> tmp;
                SAVA_REMOTE_MSGS[id] = tmp;
            }
            for (int w = 0; w < num; w++) {
                fscanf(fp, "%lf", &value);
                SAVA_REMOTE_MSGS[id].push_back(value);
            }
        }
        fclose(fp);
    }
    plog("Receive messages files from each worker");

    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fname = "./mp4/tmp/rmsg_out_" + std::to_string(i) + ".txt";
        fp = fopen(fname.c_str(), "w");
        for (auto x : SAVA_REMOTE_MSGS) {
            if (SAVA_VERTEX_MAPPING[x.first] != i) continue;
            fprintf(fp, "%d %lu", x.first, x.second.size());
            for (int w = 0; w < x.second.size(); w++) {
                fprintf(fp, " %lf", x.second[w]);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
    plog("Generate messages files for each worker");
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fname = "./mp4/tmp/rmsg_out_" + std::to_string(i) + ".txt";
        SAVA_WORKER_CONN[i]->sendFile(fname.c_str());
    }
    plog("Send messages files to each worker");
    
    count = 0;
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        SAVA_WORKER_CONN[i]->recvStr(ack);
        plog("Recv %s from %d", ack.c_str(), i);
        count += stoi(ack);
    }
    plog("Active nodes after this round is %d", count);
    if (count == 0) SAVA_STATE = 2;
    return 0;
}

