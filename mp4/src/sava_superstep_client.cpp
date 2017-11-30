#include "daemon.hpp"

void Daemon::savaInitPregelClient() {
    FILE *fp;
    string fname;
    int id, src, num, dst;
    double value;

    SAVA_NUM_MSG = 0;
    SAVA_VERTEX_MAPPING.clear();
    SAVA_WORKER_MAPPING.clear();
    PREGEL_IN_MESSAGES.clear();
    PREGEL_OUT_MESSAGES.clear();
    PREGEL_LOCAL_VERTICES.clear();
    PREGEL_LOCAL_EDGES.clear();

    fname = "./mp4/sava/vertices.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d", &id) != EOF) {
        PREGEL_LOCAL_VERTICES[id] = 0.0;
    }
    fclose(fp);
    plog("init vertices");
    
    fname = "./mp4/sava/edges.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d %d", &src, &num) != EOF) {
        if (num == 0) continue;
        vector<Edge> tmp;
        PREGEL_LOCAL_EDGES[src] = tmp;
        for (int i = 0; i < num; i++) {
            fscanf(fp, "%d %lf", &dst, &value); 
            Edge edg(src, dst, value);
            PREGEL_LOCAL_EDGES[src].push_back(edg);
        }
    }
    fclose(fp);
    plog("init edges");

    fname = "./mp4/sava/fullvmap.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d %d", &src, &dst) != EOF) {
        SAVA_VERTEX_MAPPING[src] = dst;
    }
    fclose(fp);
    plog("init vertex mappings: %d vertices.", SAVA_VERTEX_MAPPING.size());

    fname = "./mp4/sava/fullwmap.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d %d", &src, &dst) != EOF) {
        SAVA_WORKER_MAPPING[src] = dst;
        plog("worker %d -> member %d", src, dst);
    }
    fclose(fp);
    SAVA_NUM_WORKER = SAVA_WORKER_MAPPING.size();
    plog("init worker mappings");
        
    SAVA_ROUND = 0;
}

void Daemon::pregelInitStep() {
    SAVA_NUM_MSG = 0;
    SAVA_WORKER_CONN.clear();


    for (auto x : SAVA_WORKER_MAPPING) {
        if (x.second == self_index) {
            SAVA_WORKER_ID = x.first;
        }
    }

    plog("Worker ID is %d", SAVA_WORKER_ID);
    for (auto x : SAVA_WORKER_MAPPING) {
        if (x.second == self_index) {
            SAVA_WORKER_CONN[x.first] = NULL;
        } else {
            SAVA_WORKER_CONN[x.first] = new TCPSocket(member_list[x.second].ip, BASEPORT + 4);
            string cmd = "savaworkermsgs;" + to_string(SAVA_WORKER_ID);
            SAVA_WORKER_CONN[x.first]->sendStr(cmd);
            plog(cmd.c_str());
        }
    }
}

void Daemon::pregelWriteVertices() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/vertices_" + std::to_string(SAVA_ROUND) + ".txt";
    fp = fopen(fname.c_str(), "w");
    if (SAVA_ROUND == 0) {
        for (auto x : PREGEL_LOCAL_VERTICES) {
            fprintf(fp, "%d %lf\n", x.first, x.second);
        }
    } else {
        for (auto x : PREGEL_LOCAL_VERTICES) {
            if (PREGEL_IN_MESSAGES.find(x.first) == PREGEL_IN_MESSAGES.end()) continue;
            if (PREGEL_IN_MESSAGES[x.first].size() == 0) continue;
            fprintf(fp, "%d %lf\n", x.first, x.second);
        }
    }
    fclose(fp);
}

void Daemon::pregelWriteEdges() {
    FILE *fp;
    string fname;
    
    fname = "./mp4/sava/edges_" + std::to_string(SAVA_ROUND) + ".txt";
    fp = fopen(fname.c_str(), "w");
    if (SAVA_ROUND == 0) {
        for (auto x : PREGEL_LOCAL_VERTICES) {
            int id = x.first;
            if (PREGEL_LOCAL_EDGES.find(id) == PREGEL_LOCAL_EDGES.end()) continue;
            fprintf(fp, "%d %lu", id, PREGEL_LOCAL_EDGES[id].size());
            for (size_t i = 0; i < PREGEL_LOCAL_EDGES[id].size(); i++) {
                fprintf(fp, " %d %lf", PREGEL_LOCAL_EDGES[id][i].Dst(), PREGEL_LOCAL_EDGES[id][i].Value());
            }
            fprintf(fp, "\n");
        }
    } else {
        for (auto x : PREGEL_LOCAL_VERTICES) {
            if (PREGEL_IN_MESSAGES.find(x.first) == PREGEL_IN_MESSAGES.end()) continue;
            if (PREGEL_IN_MESSAGES[x.first].size() == 0) continue;
            int id = x.first;
            if (PREGEL_LOCAL_EDGES.find(id) == PREGEL_LOCAL_EDGES.end()) continue;
            fprintf(fp, "%d %lu", id, PREGEL_LOCAL_EDGES[id].size());
            for (size_t i = 0; i < PREGEL_LOCAL_EDGES[id].size(); i++) {
                fprintf(fp, " %d %lf", PREGEL_LOCAL_EDGES[id][i].Dst(), PREGEL_LOCAL_EDGES[id][i].Value());
            }
            fprintf(fp, "\n");
        }
    }
    fclose(fp);    
}

void Daemon::pregelWriteMessages() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/messages_" + std::to_string(SAVA_ROUND) + ".txt";
    fp = fopen(fname.c_str(), "w");
    for (auto x : PREGEL_IN_MESSAGES) {
        if (x.second.size() == 0) continue;
        fprintf(fp, "%d %lu", x.first, x.second.size());
        for (size_t i = 0; i < x.second.size(); i++) {
            fprintf(fp, " %lf", x.second[i].Value());
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Daemon::pregelExecution() {
    string cmd = "./mp4/sava/runner " + SAVA_APP_NAME + " " + to_string(SAVA_ROUND) + " " + to_string(SAVA_NUM_VERTICES) + " " + SAVA_COMBINATOR;
    system(cmd.c_str());
}

void Daemon::pregelReadVertices() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/newvtxs_"+ std::to_string(SAVA_ROUND) + ".txt";
    fp = fopen(fname.c_str(), "r");
    if (fp == NULL) {
        plog("readVertices file is null");
    }
    int id;
    double value;
    while (fscanf(fp, "%d %lf", &id, &value) != EOF) {
        PREGEL_LOCAL_VERTICES[id] = value;
    }
    fclose(fp);
}

void Daemon::pregelReadLocalMessages() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/newmsgs_" + std::to_string(SAVA_ROUND) + ".txt";
    fp = fopen(fname.c_str(), "r");
    int id, num;
    double value;
    PREGEL_IN_MESSAGES.clear();
    PREGEL_OUT_MESSAGES.clear();
    while (fscanf(fp, "%d %d", &id, &num) != EOF) {
        if (PREGEL_LOCAL_VERTICES.find(id) == PREGEL_LOCAL_VERTICES.end()) {
            if (PREGEL_OUT_MESSAGES.find(id) == PREGEL_OUT_MESSAGES.end()) {
                vector<Message> tmp;
                PREGEL_OUT_MESSAGES[id] = tmp;
            }
            for (int i = 0; i < num; i++) {
                fscanf(fp, "%lf", &value);
                PREGEL_OUT_MESSAGES[id].push_back(value);
            }
        } else {
            if (PREGEL_IN_MESSAGES.find(id) == PREGEL_IN_MESSAGES.end()) {
                vector<Message> tmp;
                PREGEL_IN_MESSAGES[id] = tmp;
            }
            for (int i = 0; i < num; i++) {
                fscanf(fp, "%lf", &value);
                PREGEL_IN_MESSAGES[id].push_back(value);
            }
        }
    }
    fclose(fp);
}

void Daemon::pregelCombineMessages() {
    if (SAVA_COMBINATOR == "MIN") {
        for (auto v : PREGEL_OUT_MESSAGES) {
            if (v.second.size() == 0) continue;
            double minv = v.second[0].Value();
            for (auto w : v.second) {
                minv = min(minv, w.Value());
            }
            v.second.clear();
            v.second.push_back(Message(minv));
        }
    } else if (SAVA_COMBINATOR == "MAX") {
        for (auto v : PREGEL_OUT_MESSAGES) {
            if (v.second.size() == 0) continue;
            double maxv = v.second[0].Value();
            for (auto w : v.second) {
                maxv = max(maxv, w.Value());
            }
            v.second.clear();
            v.second.push_back(Message(maxv));
        }
    } else if (SAVA_COMBINATOR == "SUM") {
        for (auto v : PREGEL_OUT_MESSAGES) {
            if (v.second.size() == 0) continue;
            double sumv = 0;
            for (auto w : v.second) {
                sumv = sumv + w.Value();
            }
            v.second.clear();
            v.second.push_back(Message(sumv));
        }
    } else {
        return;
    }

}

void Daemon::pregelGenRemoteMessages() {

    FILE *fps[SAVA_NUM_WORKER];
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        if (i == SAVA_WORKER_ID) continue;
        string fname = "./mp4/sava/outmsgs_" + to_string(i) + ".txt";
        fps[i-1] = fopen(fname.c_str(), "w");
    }

    for (auto x : PREGEL_OUT_MESSAGES) {
        int idx = SAVA_VERTEX_MAPPING[x.first]-1;
        fprintf(fps[idx], "%d %lu", x.first, x.second.size());
        for (size_t i = 0; i < x.second.size(); i++) {
            fprintf(fps[idx], " %f", x.second[i].Value());
        }
        fprintf(fps[idx], "\n");
    }

    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        if (i == SAVA_WORKER_ID) continue;
        fclose(fps[i-1]);
        string fname = "./mp4/sava/outmsgs_" + to_string(i) + ".txt";
        SAVA_WORKER_CONN[i]->sendFile(fname);
    }
}

void Daemon::pregelReadRemoteMessages() {
    FILE *fp;
    string fname;
    int cnt, total_cnt = 0;
    while (SAVA_NUM_MSG != (SAVA_NUM_WORKER - 1));
    for (int i = 1; i <= SAVA_NUM_WORKER; i++) {
        if (i == SAVA_WORKER_ID) continue;
        fname = "./mp4/sava/rmsgs_" + std::to_string(i) + ".txt";
        fp = fopen(fname.c_str(), "r");
        if (fp == NULL) continue;
        int id, num;
        double value;
        cnt = 0;
        while (fscanf(fp, "%d %d", &id, &num) != EOF) {
            total_cnt++;
            if (PREGEL_LOCAL_VERTICES.find(id) == PREGEL_LOCAL_VERTICES.end()) {
                // Actually it is exception
                plog("Error in rmsg vtx %d", id);
                continue;
            } else {
                cnt++;
                if (PREGEL_IN_MESSAGES.find(id) == PREGEL_IN_MESSAGES.end()) {
                    vector<Message> tmp;
                    PREGEL_IN_MESSAGES[id] = tmp;
                }
                for (int i = 0; i < num; i++) {
                    fscanf(fp, "%lf", &value);
                    PREGEL_IN_MESSAGES[id].push_back(value);
                }
            }
        }
        plog("Read %s's %d msgs", fname.c_str(), cnt);
        fclose(fp);
    }
    plog("Read %d msgs in total.", total_cnt);
}

int Daemon::savaClientSuperstep(TCPSocket *sock, int step) {
    string fname;
    SAVA_ROUND = step;
    plog("ROUND: %d", SAVA_ROUND);
    plog("Using combinator: %s", SAVA_COMBINATOR.c_str());
    pregelInitStep();
    pregelWriteEdges();
    pregelWriteMessages();
    pregelWriteVertices();
    pregelExecution();
    pregelReadVertices();
    pregelReadLocalMessages();
    pregelCombineMessages();
    pregelGenRemoteMessages();
    pregelReadRemoteMessages();
    system("mv ./mp4/sava/rmsgs_*.txt ./mp4/tmp/rmsgs_*.txt");
    system("mv ./mp4/sava/outmsgs_*.txt ./mp4/tmp/outmsgs_*.txt");
    sock->sendStr(to_string(PREGEL_IN_MESSAGES.size()));
    return 0;
}
