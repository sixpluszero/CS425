#include "exec.hpp"
int SUPERSTEP;
string APP_NAME, COMBINE;
map<int, vector<Message> > VERTEX_IN_MESSAGES, VERTEX_NEXT_MESSAGES;
map<int, vector<Edge> > VERTEX_EDGES;
vector<Vertex> ACTIVE_VERTICES;
map<int, double> VERTEX_NEXT_VALUE;

void init(int argc, char *argv[]) {
    assert(argc > 2);
    APP_NAME = string(argv[1]);
    SUPERSTEP = atoi(argv[2]);
    if (argc == 4) {
        COMBINE = string(argv[3]);
    }
}

void readVertices() {
    string fname = APP_NAME + "_" + std::to_string(SUPERSTEP) + "_" + "vertices.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    int id;
    double value;
    while (fscanf(fp, "%d %lf", &id, &value) != EOF) {
        //printf("%d %lf\n", id, value);
        Vertex tmp(id, value);
        ACTIVE_VERTICES.push_back(tmp);
    }
    fclose(fp);
}

void readMessages() {
    string fname = APP_NAME + "_" + std::to_string(SUPERSTEP) + "_" + "messages.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    int id, num;
    double value;
    while (fscanf(fp, "%d %d", &id, &num) != EOF) {
        if (num == 0) continue;
        if (VERTEX_IN_MESSAGES.find(id) == VERTEX_IN_MESSAGES.end()) {
            vector<Message> tmp;
            VERTEX_IN_MESSAGES[id] = tmp;
        }
        for (int i = 0; i < num; i++) {
            fscanf(fp, "%lf", &value);
            //printf("msg: %d %lf\n", id, value);
            Message msg(value);
            VERTEX_IN_MESSAGES[id].push_back(msg);
        }
    }
    fclose(fp);
}

void readEdges() {
    string fname = APP_NAME + "_" + std::to_string(SUPERSTEP) + "_" + "edges.txt";
    FILE *fp = fopen(fname.c_str(), "r");
    if (fp == NULL) return;
    int src, num, dst;
    double value;
    while (fscanf(fp, "%d %d", &src, &num) != EOF) {
        if (num == 0) continue;
        if (VERTEX_EDGES.find(src) == VERTEX_EDGES.end()) {
            vector<Edge> tmp;
            VERTEX_EDGES[src] = tmp;
        }
        for (int i = 0; i < num; i++) {
            fscanf(fp, "%d %lf", &dst, &value);
            //printf("edge: %d %d %lf\n", src, dst, value);
            Edge edg(src, dst, value);
            VERTEX_EDGES[src].push_back(edg);
        }
    }
    fclose(fp);
}

void execution() {
    for (auto v : ACTIVE_VERTICES) {
        double org_val = v.Value();
        v.Compute(v.GetMessages());
        double new_val = v.Value();
        if (org_val != new_val) {
            VERTEX_NEXT_VALUE[v.VertexID()] = new_val;
        }
    }
    printf("Run %lu nodes\n", ACTIVE_VERTICES.size());
}

void output() {
    
    FILE *fp;
    string fname;

    // Need to implement message combinator
    fname = APP_NAME + "_" + std::to_string(SUPERSTEP) + "_" + "messages_next.txt";
    fp = fopen(fname.c_str(), "w");
    for (auto v : VERTEX_NEXT_MESSAGES) {
        if (v.second.size() == 0) continue;
        fprintf(fp, "%d %lu", v.first, v.second.size());
        for (auto m  : v.second) {
            fprintf(fp, " %lf", m.Value());
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    fname = APP_NAME + "_" + std::to_string(SUPERSTEP) + "_" + "vertices_next.txt";
    fp = fopen(fname.c_str(), "w");
    for (auto v : VERTEX_NEXT_VALUE) {
        fprintf(fp, "%d %lf\n", v.first, v.second);
    }
    fclose(fp);

}

int main(int argc, char *argv[]) {
    /**
     * Basic workflow:
     * (1) If the SUPERSTEP != 0, Read the messages from file
     * (2) If the SUPERSTEP == 0, read the vertices from file
     * (3) Read the edges from file
     * (4) Execute the UDF Compute() to computer over all the nodes with message
     * (5) Write output message to file
     * (6) Write active vertex list to file
     */

    init(argc, argv);
    readVertices();
    readMessages();
    readEdges();
    execution();
    output();

    return 0;
}