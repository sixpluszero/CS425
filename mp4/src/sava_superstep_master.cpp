#include "daemon.hpp"

void Daemon::savaInitPregelMaster() {
    string ack, cmd;
    FILE *fp;

    SAVA_WORKER_CONN.clear();
    SAVA_NUM_VERTICES = SAVA_VERTEX_MAPPING.size();
    
    cmd = "savaclientinit;" + to_string(SAVA_NUM_VERTICES);
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        SAVA_WORKER_CONN[i] = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
    }

    fp = fopen("./mp4/tmp/vmap.txt", "w");
    for (auto x : SAVA_VERTEX_MAPPING) {
        fprintf(fp, "%d %d\n", x.first, x.second);
    }
    fclose(fp);

    fp = fopen("./mp4/tmp/wmap.txt", "w");
    for (auto x : SAVA_WORKER_MAPPING) {
        fprintf(fp, "%d %d\n", x.first, x.second);
    }
    fclose(fp);

    for (auto x : SAVA_WORKER_CONN) {
        x.second->sendStr(cmd);
        x.second->recvStr(ack);
        x.second->sendFile("./mp4/tmp/wmap.txt");
        x.second->recvStr(ack);
        x.second->sendFile("./mp4/tmp/vmap.txt");
    }

    for (auto x : SAVA_WORKER_CONN) {
        x.second->recvStr(ack);
        plog("Recv init ack from worker %d", x.first);
    }
}

int Daemon::savaMasterSuperstep() {
    /**
     * (1) Send savaclientstep with step number parameter to worker.
     * (2) Wait for execution finished notification
     * (3) Request for outgoing msgs
     * (4) Distributed msgs to target worker
     * (5) Receive active node number from each worker
     * (6) Decide whether to stop
     * (7) Gather result
     */
    string fname, msg, ack;
    int count, ret;

    ret = 0;
    SAVA_WORKER_CONN.clear();
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        SAVA_WORKER_CONN[i] = new TCPSocket(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);
        SAVA_WORKER_CONN[i]->sendStr("savaclientstep;"+std::to_string(SAVA_ROUND));
    }

    count = 0;
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        if (SAVA_WORKER_CONN[i]->recvStr(ack)) {
            plog("Error in connection with %d", i);
            ret = 1;
        } else {
            plog("Recv %s active nodes from %d", ack.c_str(), i);
            count += stoi(ack);
        }
    }
    plog("Active nodes after this round is %d", count);
    if (count == 0) SAVA_STATE = 2;

    return ret;
}

