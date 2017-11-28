#include "daemon.hpp"

void Daemon::savaInitPregelClient() {
    FILE *fp;
    string fname;
    int id, src, num, dst;
    double value;

    PREGEL_IN_MESSAGES.clear();
    PREGEL_OUT_MESSAGES.clear();
    PREGEL_LOCAL_VERTICES.clear();
    fname = "./mp4/sava/vertices.txt";
    fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d", &id) != EOF) {
        PREGEL_LOCAL_VERTICES[id] = 0.0;
    }
    fclose(fp);
    plog("init vertices");
    
    PREGEL_LOCAL_EDGES.clear();
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
    SAVA_ROUND = 0;
}

void Daemon::pregelWriteVertices() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "vertices.txt";
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
    
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "edges.txt";
    fp = fopen(fname.c_str(), "w");
    if (SAVA_ROUND == 0) {
        for (auto x : PREGEL_LOCAL_VERTICES) {
            int id = x.first;
            if (PREGEL_LOCAL_EDGES.find(id) == PREGEL_LOCAL_EDGES.end()) continue;
            fprintf(fp, "%d %lu", id, PREGEL_LOCAL_EDGES[id].size());
            for (int i = 0; i < PREGEL_LOCAL_EDGES[id].size(); i++) {
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
            for (int i = 0; i < PREGEL_LOCAL_EDGES[id].size(); i++) {
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
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "messages.txt";
    fp = fopen(fname.c_str(), "w");
    for (auto x : PREGEL_IN_MESSAGES) {
        if (x.second.size() == 0) continue;
        fprintf(fp, "%d %lu", x.first, x.second.size());
        for (int i = 0; i < x.second.size(); i++) {
            fprintf(fp, " %lf", x.second[i].Value());
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Daemon::pregelExecution() {
    string cmd = "./mp4/sava/runner " + SAVA_APP_NAME + " " + to_string(SAVA_ROUND);
    system(cmd.c_str());
}

void Daemon::pregelReadVertices() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "vertices_next.txt";
    fp = fopen(fname.c_str(), "r");
    if (fp == NULL) {
        plog("readVertices  it is null");
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
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "messages_next.txt";
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

void Daemon::pregelGenRemoteMessages() {
    FILE *fp;
    string fname;
    fname = "./mp4/sava/outmsgs.txt";
    fp = fopen(fname.c_str(), "w");
    for (auto x : PREGEL_OUT_MESSAGES) {
        fprintf(fp, "%d %lu", x.first, x.second.size());
        for (int i = 0; i < x.second.size(); i++) {
            fprintf(fp, " %f", x.second[i].Value());
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Daemon::pregelReadRemoteMessages() {
    FILE *fp;
    string fname;

    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "messages_next_remote.txt";
    fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    int id, num;
    double value;
    while (fscanf(fp, "%d %d", &id, &num) != EOF) {
        if (PREGEL_LOCAL_VERTICES.find(id) == PREGEL_LOCAL_VERTICES.end()) {
            // Actually it is exception
            continue;
        } else {
            if (PREGEL_IN_MESSAGES.find(id) == PREGEL_IN_MESSAGES.end()) {
                vector<Message> tmp;
                PREGEL_IN_MESSAGES[id] = tmp;
            }
            for (int i = 0; i < num; i++) {
                fscanf(fp, "%lf", &value);
                PREGEL_IN_MESSAGES[id].push_back(value);
            }
            plog("%d's %d msg read.", id, num); 
        }

    }
    fclose(fp);
}

int Daemon::savaClientSuperstep(TCPSocket *sock, int step) {
    string fname;
    SAVA_ROUND = step;
    plog("ROUND: %d", SAVA_ROUND);
    pregelWriteEdges();
    pregelWriteMessages();
    pregelWriteVertices();
    plog("Finish writing files");
    pregelExecution();
    plog("Finish execution");
    pregelReadVertices();
    pregelReadLocalMessages();
    pregelGenRemoteMessages();
    plog("Finish update local");
    fname = "./mp4/sava/outmsgs.txt";
    if (sock->sendFile(fname.c_str()) == 0) {
        plog("Finish send msgs");
        system("rm ./mp4/sava/outmsgs.txt");
    } else {
        plog("Error in sending msgs");
        return 1;
    }
    
    fname = "./mp4/sava/" + SAVA_APP_NAME + "_" + std::to_string(SAVA_ROUND) + "_" + "messages_next_remote.txt";
    if (sock->recvFile(fname.c_str()) == 0) {
        plog("Finish recv msgs");
    } else {
        plog("Error in receiving msgs");
        return 1;
    }
    pregelReadRemoteMessages();
    plog("Finish update remote");
    
    sock->sendStr(to_string(PREGEL_IN_MESSAGES.size()));
    return 0;
}
