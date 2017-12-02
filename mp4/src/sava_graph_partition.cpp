#include "daemon.hpp"

int Daemon::savaPartitionGraph() {
    FILE *fp;
    string ack, cmd, fname;
    for (auto x : file_location[SAVA_INPUT]) {
        TCPSocket sock_f(member_list[x.first].ip, BASEPORT + 3);
        cmd = "fileget;" + SAVA_INPUT;
        sock_f.sendStr(cmd.c_str());
        if (sock_f.recvFile("./mp4/tmp/tmpgraph") == 0) {
            plog("graph file received.");
            break;
        }
    }

    SAVA_WORKER_MAPPING.clear();
    SAVA_VERTEX_MAPPING.clear();
    SAVA_EDGES.clear();
    SAVA_NUM_WORKER = 0;
    for (auto x : member_list) {
        if (master_list.find(x.first) != master_list.end()) continue;
        SAVA_WORKER_MAPPING[++SAVA_NUM_WORKER] = x.first;
    }
    plog("Worker number: %d", SAVA_NUM_WORKER);

    fp = fopen("./mp4/tmp/tmpgraph", "r");
    int x, y;
    srand(time(NULL));
    while (fscanf(fp, "%d %d", &x, &y) != EOF) {
        if (SAVA_VERTEX_MAPPING.find(x) == SAVA_VERTEX_MAPPING.end()) {
            SAVA_VERTEX_MAPPING[x] = rand() % SAVA_NUM_WORKER + 1;
            //SAVA_VERTEX_MAPPING[x] = x % SAVA_NUM_WORKER + 1;
        }
        if (SAVA_VERTEX_MAPPING.find(y) == SAVA_VERTEX_MAPPING.end()) {
            SAVA_VERTEX_MAPPING[y] = rand() % SAVA_NUM_WORKER + 1;
            //SAVA_VERTEX_MAPPING[y] = y % SAVA_NUM_WORKER + 1;
        }

        if (SAVA_EDGES.find(x) == SAVA_EDGES.end()) {
            vector<int> tmp;
            SAVA_EDGES[x] = tmp;
        }
        SAVA_EDGES[x].push_back(y);

        if (SAVA_GRAPH == 1) continue;
        
        if (SAVA_EDGES.find(y) == SAVA_EDGES.end()) {
            vector<int> tmp;
            SAVA_EDGES[y] = tmp;
        }
        SAVA_EDGES[y].push_back(x);
        
    }
    fclose(fp);

    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        fp = fopen("./mp4/tmp/part_v.txt", "w");
        for (auto x : SAVA_VERTEX_MAPPING) {
            if (x.second != i) continue;
            fprintf(fp, "%d\n", x.first);
        }
        fclose(fp);
        
        fp = fopen("./mp4/tmp/part_e.txt", "w");
        for (auto x : SAVA_EDGES) {
            if (SAVA_VERTEX_MAPPING[x.first] != i) continue;
            fprintf(fp, "%d %lu", x.first, x.second.size());
            for (auto w : x.second) {
                fprintf(fp, " %d %lf", w, 1.0);
            }
            fprintf(fp, "\n");
        }
        fclose(fp);

        TCPSocket sock_w(member_list[SAVA_WORKER_MAPPING[i]].ip, BASEPORT + 4);

        cmd = "savasendinput;";
        sock_w.sendStr(cmd.c_str());
        sock_w.recvStr(ack);
        fname = "./mp4/tmp/part_v.txt";
        sock_w.sendFile(fname.c_str());
        sock_w.recvStr(ack);
        fname = "./mp4/tmp/part_e.txt";
        sock_w.sendFile(fname.c_str());
        sock_w.recvStr(ack);
        fname = "./mp4/sava/runner";
        sock_w.sendFile(fname.c_str());
        sock_w.recvStr(ack);
        cmd = SAVA_APP_NAME + ";" + SAVA_COMBINATOR;
        sock_w.sendStr(cmd.c_str());
        sock_w.recvStr(ack);
        plog("Distributed file to %d(%s)", SAVA_WORKER_MAPPING[i], member_list[SAVA_WORKER_MAPPING[i]].ip.c_str());
    }
    return 0;
}