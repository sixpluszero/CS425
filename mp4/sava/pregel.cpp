#include "exec.hpp"
#include "tcpsocket.hpp"
int SUPERSTEP, NUM_VERTICES, WORKER_ID, NUM_WORKER, RES_TYPE, RES_NUM;
string APP_NAME, COMBINE;
bool EXIT_FLAG;
map<int, int> VERTEX_MAPPING;
map<int, vector<Message> > VERTEX_IN_MESSAGES, VERTEX_NEXT_MESSAGES;
map<int, vector<Edge> > EDGES;
vector<Vertex> VERTICES;
TCPServerSocket pregel_socket(9999);

bool prefixMatch(string org, string patt){
    if (org.length() < patt.length()) return false;
    return org.substr(0, patt.length()) == patt;
}

void readVertexMapping() {
    VERTEX_MAPPING.clear();
    int src, dst;
    string fname = "./mp4/sava/fullvmap.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    while (fscanf(fp, "%d %d", &src, &dst) != EOF) {
        VERTEX_MAPPING[src] = dst;
    }
}

void readVertices() {
    int id;
    string fname = "./mp4/sava/vertices.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    while (fscanf(fp, "%d", &id) != EOF) {
        VERTICES.push_back(Vertex(id, 0.0));
    }
    fclose(fp);
}

void readEdges() {
    int src, num, dst;
    double value;
    string fname = "./mp4/sava/edges.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    while (fscanf(fp, "%d %d", &src, &num) != EOF) {
        if (num == 0) continue;
        if (EDGES.find(src) == EDGES.end()) {
            vector<Edge> tmp;
            EDGES[src] = tmp;
        }
        for (int i = 0; i < num; i++) {
            fscanf(fp, "%d %lf", &dst, &value);
            EDGES[src].push_back(Edge(src, dst, value));
        }
    }
    fclose(fp);
}

void readMessages() {
    int id, num;
    double value;
    string fname = "./mp4/sava/messages_" + std::to_string(SUPERSTEP) + ".txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    while (fscanf(fp, "%d %d", &id, &num) != EOF) {
        if (num == 0) continue;
        if (VERTEX_IN_MESSAGES.find(id) == VERTEX_IN_MESSAGES.end()) {
            vector<Message> tmp;
            VERTEX_IN_MESSAGES[id] = tmp;
        }
        for (int i = 0; i < num; i++) {
            fscanf(fp, "%lf", &value);
            Message msg(value);
            VERTEX_IN_MESSAGES[id].push_back(msg);
        }
    }
    fclose(fp);
}

void execution() {
    for (int i = 0; i < int(VERTICES.size()); i++) {
        VERTICES[i].Compute();
    }
    FILE *fp = fopen("./runner.log", "a");
    fprintf(fp, "nmsg num is %lu\n", VERTEX_NEXT_MESSAGES.size());
    fclose(fp);
}

void combineMessages() {
    if (COMBINE == "MIN") {
        for (auto v : VERTEX_NEXT_MESSAGES) {
            if (v.second.size() == 0) continue;
            double minv = v.second[0].Value();
            for (auto w : v.second) {
                minv = min(minv, w.Value());
            }
            v.second.clear();
            v.second.push_back(Message(minv));
        }
    } else if (COMBINE == "MAX") {
        for (auto v : VERTEX_NEXT_MESSAGES) {
            if (v.second.size() == 0) continue;
            double maxv = v.second[0].Value();
            for (auto w : v.second) {
                maxv = max(maxv, w.Value());
            }
            v.second.clear();
            v.second.push_back(Message(maxv));
        }
    } else if (COMBINE == "SUM") {
        for (auto v : VERTEX_NEXT_MESSAGES) {
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

void writeMessages() {
    VERTEX_IN_MESSAGES.clear();
    FILE *fps[NUM_WORKER];
    for (int i = 1; i <= NUM_WORKER; i++) {
        if (i == WORKER_ID) continue;
        string fname = "./mp4/sava/outmsgs_" + to_string(i) + ".txt";
        fps[i-1] = fopen(fname.c_str(), "w");
    }
    for (auto v : VERTEX_NEXT_MESSAGES) {
        if (v.second.size() == 0) continue;
        int idx = VERTEX_MAPPING[v.first] - 1;
        if (idx + 1 == WORKER_ID) {
            VERTEX_IN_MESSAGES[v.first] = v.second;
            continue;
        }
        fprintf(fps[idx], "%d %lu", v.first, v.second.size());
        for (auto m : v.second) {
            fprintf(fps[idx], " %lf", m.Value());
        }
        fprintf(fps[idx], "\n");
    }

    for (int i = 1; i <= NUM_WORKER; i++) {
        if (i == WORKER_ID) continue;
        fclose(fps[i-1]);
    }

    VERTEX_NEXT_MESSAGES.clear();
    SUPERSTEP++;
}

void readRemoteMsgs() {
    FILE *fp;
    string fname;
    for (int i = 1; i <= NUM_WORKER; i++) {
        if (i == WORKER_ID) continue;
        fname = "./mp4/sava/rmsgs_" + std::to_string(i) + ".txt";
        fp = fopen(fname.c_str(), "r");
        if (fp == NULL) continue;
        int id, num;
        double value;
        while (fscanf(fp, "%d %d", &id, &num) != EOF) {
            if (VERTEX_IN_MESSAGES.find(id) == VERTEX_IN_MESSAGES.end()) {
                vector<Message> tmp;
                VERTEX_IN_MESSAGES[id] = tmp;
            }
            for (int i = 0; i < num; i++) {
                fscanf(fp, "%lf", &value);
                VERTEX_IN_MESSAGES[id].push_back(value);
            }
        }
        fclose(fp);
    }
}

void init() {
    FILE *fp = fopen("./runner.log", "a");
    fprintf(fp, "entering init()\n");
    readVertexMapping();
    fprintf(fp, "finish readVertexMapping()\n");
    readVertices();
    fprintf(fp, "finish readVertices()\n");
    readEdges();
    fprintf(fp, "finish readEdges()\n");
    NUM_VERTICES = VERTEX_MAPPING.size();
    SUPERSTEP = 0;
    EXIT_FLAG = false;
    fprintf(fp, "%d %d %d %s %d\n", SUPERSTEP, WORKER_ID, NUM_WORKER, COMBINE.c_str(), NUM_VERTICES);
    fclose(fp);
}

void superstep() {
    FILE *fp = fopen("./runner.log", "a");
    fprintf(fp, "entering execution %d\n", SUPERSTEP);
    fclose(fp);
    execution();
    fp = fopen("./runner.log", "a");
    fprintf(fp, "finish execution %d\n", SUPERSTEP);
    fclose(fp);
    combineMessages();
    fp = fopen("./runner.log", "a");
    fprintf(fp, "finish combineMessages %d\n", SUPERSTEP);
    fclose(fp);
    writeMessages();
    fp = fopen("./runner.log", "a");
    fprintf(fp, "finish writeMessages %d\n", SUPERSTEP);
    fclose(fp);
}

void writeResults() {
    priority_queue< KV, vector<KV>, greater<KV> > minHeap;
    priority_queue< KV > maxHeap;
    FILE *fp;
    int cnt = 0;
    if (RES_TYPE == 0) {
        fp = fopen("./mp4/tmp/localres", "w");
        for (auto x : VERTICES) {
            fprintf(fp, "%d %lf\n", x.VertexID(), x.Value());
        }
        fclose(fp);
        return;
    }

    if (RES_TYPE == 1) {
        for (auto x : VERTICES) {
            minHeap.push(KV(x.VertexID(), x.Value()));
        }
        fp = fopen("./mp4/tmp/localres", "w");
        while (!minHeap.empty()) {
            fprintf(fp, "%d %lf\n", minHeap.top().id, minHeap.top().value);
            cnt++;
            if (cnt == RES_NUM) break;
            minHeap.pop();
        }
        fclose(fp);
        return;
    }

    if (RES_TYPE == 2) {
        for (auto x : VERTICES) {
            maxHeap.push(KV(x.VertexID(), x.Value()));
        }
        fp = fopen("./mp4/tmp/localres", "w");
        while (!maxHeap.empty()) {
            fprintf(fp, "%d %lf\n", maxHeap.top().id, maxHeap.top().value);
            cnt++;
            if (cnt == RES_NUM) break;
            maxHeap.pop();
        }
        fclose(fp);
        return;
    }
}

void msgHandler(TCPSocket *sock) {
    string info, cmd;
    int r;
    r = sock->recvStr(info);
    if (r == 1) {
        printf("Error in client. Close the connection.");
        delete sock;
        return;
    }
    FILE *fp = fopen("./runner.log", "a");
    fprintf(fp, "debug: %s\n", info.c_str());
    fclose(fp);
    if (prefixMatch(info, "init")) {
        init();
        sock->sendStr("ack");
    } else if (prefixMatch(info, "superstep")) {
        FILE *fp = fopen("./runner.log", "a");
        fprintf(fp, "enter superstep\n");
        fclose(fp);        
        superstep();
        if (sock->sendStr("ack")) {
            FILE *fp2 = fopen("./runner.log", "a");
            fprintf(fp2, "sendback ack problem\n");
            fclose(fp2);
        } else {
            FILE *fp2 = fopen("./runner.log", "a");
            fprintf(fp2, "sendback ack no problem\n");
            fclose(fp2);
        }
    } else if (prefixMatch(info, "remotemsg")) {
        readRemoteMsgs();
        sock->sendStr("ack");
    } else if (prefixMatch(info, "result")) {
        info = info.substr(info.find(";")+1, info.length());
        int RES_TYPE = stoi(info.substr(0, info.find(";")));
        info = info.substr(info.find(";")+1, info.length());
        int RES_NUM = stoi(info.substr(0, info.find(";")));
        writeResults();
        sock->sendStr("ack");
    } else if (prefixMatch(info, "active")) {
        FILE *fp = fopen("./runner.log", "a");
        fprintf(fp, "active node is %lu\n", VERTEX_IN_MESSAGES.size());
        fclose(fp);
        sock->sendStr(to_string(VERTEX_IN_MESSAGES.size()));
    } else if (prefixMatch(info, "stop")) {
        EXIT_FLAG = true;
    }
}

void receiver() {
    while (EXIT_FLAG == false) {
        for (;;) {
            TCPSocket *sock = pregel_socket.accept();
            msgHandler(sock);
            FILE *fp = fopen("./runner.log", "a");
            fprintf(fp, "end connection\n");
            fclose(fp);
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    WORKER_ID = atoi(argv[1]);
    NUM_WORKER = atoi(argv[2]);
    COMBINE = string(argv[3]);
    
    /*
    auto start = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> dif = end - start;
    printf("read vtx takes %lf sec\n", dif.count());
    */
    receiver();
    return 0;
}