#include "daemon.hpp"

void Daemon::savaInitPregelClient() {
    FILE *fp;
    string fname, cmd, ack;
    int src, dst;
    SAVA_WORKER_MAPPING.clear();
    fname = "./mp4/sava/fullwmap.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d %d", &src, &dst) != EOF) {
        if (dst == self_index) SAVA_WORKER_ID = src;
        SAVA_WORKER_MAPPING[src] = dst;
        plog("worker %d -> member %d", src, dst);
    }
    plog("Self worker ID is %d", SAVA_WORKER_ID);
    fclose(fp);
    SAVA_NUM_WORKER = SAVA_WORKER_MAPPING.size();
    plog("init worker mappings"); 
        
    SAVA_ROUND = 0;

    cmd = "nohup ./mp4/sava/runner " + to_string(SAVA_WORKER_ID);
    cmd = cmd + " " + to_string(SAVA_NUM_WORKER);
    cmd = cmd + " " + SAVA_COMBINATOR;
    cmd = cmd + " > /dev/null 2>&1 &";
    plog("command is %s", cmd.c_str());
    system(cmd.c_str());
    usleep(100000);
    plog("debug %s", member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip.c_str());
    TCPSocket *soc = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    soc->sendStr("init");
    soc->recvStr(ack);
    plog("start executor");
}

void Daemon::savaClientInitStep() {
    string cmd, ack;
    SAVA_WORKER_CONN.clear();
    for (auto x : SAVA_WORKER_MAPPING) {
        if (x.second == self_index) {
            SAVA_WORKER_CONN[x.first] = NULL;
        } else {
            SAVA_WORKER_CONN[x.first] = new TCPSocket(member_list[x.second].ip, BASEPORT + 4);
            cmd = "savaworkermsgs;" + to_string(SAVA_WORKER_ID) + ";" + to_string(SAVA_ROUND) + ";"; 
            //plog("try connecting to %d", x.first);
            if (SAVA_WORKER_CONN[x.first]->sendStr(cmd)) {
                plog("error in sending connection to %d", x.first);
            } 
            //plog("ok1 %d %lu", x.first, SAVA_WORKER_CONN.size());
            if (SAVA_WORKER_CONN[x.first]->recvStr(ack)) {
                plog("error in receiving ack from %d", x.first);
            }
            //plog("ok2 %d", x.first);
        }
    }
}

void Daemon::savaSuperstep() {
    string ack;
    plog("entering superstep.0");
    
    RUNNER_SOCK = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    plog("entering superstep.0.5");
    if (RUNNER_SOCK->sendStr("superstep")) {
        plog("error in superstep msg");
        return;
    } else {
        printf("pentering superstep.1\n");
        plog("entering superstep.1");
    }
    
    if (RUNNER_SOCK->recvStr(ack)) {
        plog("error in superstep recv");
        return;        
    } else {
        printf("pentering superstep.2\n");
        plog("entering superstep.2");
    }
    
}

void Daemon::savaMessageExchange() {
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        if (i == SAVA_WORKER_ID) continue;
        string fname = "./mp4/sava/outmsgs_" + to_string(i) + ".txt";
        plog("sending %s null?%d", fname.c_str(), SAVA_WORKER_CONN.find(i) == SAVA_WORKER_CONN.end());
        SAVA_WORKER_CONN[i]->sendFile(fname);
    }
    plog("Sent messages to other workers");
    while (SAVA_NUM_MSG != (SAVA_NUM_WORKER - 1));
    plog("Receive messages from other workers");
}

void Daemon::savaReadRemoteMessages() {
    string ack;
    RUNNER_SOCK = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    RUNNER_SOCK->sendStr("remotemsg");
    RUNNER_SOCK->recvStr(ack);
}

int Daemon::savaActiveNodes() {
    string ack;
    RUNNER_SOCK = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    RUNNER_SOCK->sendStr("active");
    RUNNER_SOCK->recvStr(ack);
    return stoi(ack);
}

int Daemon::savaClientSuperstep(TCPSocket *sock, int step) {
    string fname, ack;
    SAVA_NUM_MSG = 0;
    SAVA_ROUND = step;
    plog("ROUND: %d", SAVA_ROUND);
    plog("Using combinator: %s", SAVA_COMBINATOR.c_str());
    savaClientInitStep();
    plog("savaClientInitStep() finished");
    savaSuperstep();
    plog("savaSuperstep() finished");
    savaMessageExchange();
    plog("savaMessageExchange() finished");
    savaReadRemoteMessages();
    plog("savaReadRemoteMessages() finished");
    int cnt = savaActiveNodes();
    sock->sendStr(to_string(cnt));
    plog("savaActiveNodes() finished");
    system("mv ./mp4/sava/rmsgs_*.txt ./mp4/tmp/");
    system("mv ./mp4/sava/outmsgs_*.txt ./mp4/tmp/");
    return 0;
}
