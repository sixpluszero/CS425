#include "daemon.hpp"

int Daemon::savaCompile() {
    string command;
    command = "cp ./mp4/tmp/client.cpp ./mp4/sava/client.cpp";
    system(command.c_str());
    command = "g++ -O3 -std=c++11 -o ./mp4/sava/runner ./mp4/sava/client.cpp ./mp4/sava/pregel.cpp";
    try {
        system(command.c_str());
    } catch (...) {
        return 1;
    }
    return 0;
}

/* TODO: This function is buggy */
void Daemon::savaReplicateMeta() {
    string ack, fname;
    for (auto x : master_list) {
        if (x.second == "Backup") {
            string msg = "savareplicatemeta;";
            msg += SAVA_APP_NAME + ";";
            msg += SAVA_INPUT + ";";
            msg += SAVA_OUTPUT + ";";
            msg += SAVA_COMBINATOR + ";";
            plog("Try sending %s to %s", msg.c_str(), member_list[x.first].ip.c_str());
            TCPSocket sock_b(member_list[x.first].ip, BASEPORT + 4);
            if (sock_b.sendStr(msg)) {
                plog("Error in replicating meta data from primary to backup master");
                return;
            }
            sock_b.recvStr(ack);
            fname = "./mp4/sava/runner";
            if (sock_b.sendFile(fname.c_str())) {
                plog("Error in replicating meta data from primary to backup master");
                return;
            };
            sock_b.recvStr(ack);
        }
    }
}

void Daemon::savaTask(TCPSocket *sock, string app, string input, string output, string comb) {
    SAVA_APP_NAME = app;
    SAVA_INPUT = input;
    SAVA_OUTPUT = output;
    SAVA_COMBINATOR = comb;
    SAVA_ROUND = 0;
    SAVA_STATE = 0; // Finish is 2;
    SAVA_GRAPH = 0; // Default is undirected.
    if (SAVA_COMBINATOR == "") {
        SAVA_COMBINATOR = "none";
    }
    // Ask for client code
    if (sock->recvFile("./mp4/sava/client.cpp") == 1) return;
    plog("Client code received.");
    // Compile client to get the binary
    if (savaCompile()) {
        sock->sendStr("error: cannot compile source code.");
        return;
    }
    plog("Client code compiled.");
    // Replicate to standby master
    // savaReplicateMeta();
    plog("Replication metadata to standby master completed.");
    // Partiton graph
    savaPartitionGraph();
    plog("Graph partition and distribution completed.");
    
    savaInitPregelMaster();
    plog("Init worker subgraph.");
    // Start superstep computation;
    while (SAVA_STATE != 2) {
        savaMasterSuperstep();
        SAVA_ROUND++;
        plog("Finish %d round.", SAVA_ROUND);
        if (SAVA_ROUND > 8) break;
    }
    
}

void Daemon::sava() {
  while (leave_flag == false) {
    for (;;) {
      TCPSocket *sock = sava_socket.accept();
      std::thread msg_t(&Daemon::savaHandler, this, sock);
      msg_t.detach();
    }
  }
  plog("module channel exit");
}
