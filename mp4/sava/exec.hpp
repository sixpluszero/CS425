#ifndef MP4__PREGEL__HPP
#define MP4__PREGEL__HPP
#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <chrono>
#include "pregel.hpp"
using namespace std;

typedef vector<Message>& MessageIterator;
typedef vector<Edge>& EdgeIterator;

extern int SUPERSTEP, NUM_VERTICES;
extern map<int, vector<Message> > VERTEX_IN_MESSAGES, VERTEX_NEXT_MESSAGES;
extern map<int, vector<Edge> > VERTEX_EDGES;


class Vertex {
    private:
        double value;
        int id;
    public:
        Vertex(int id_, double val_) {
            value = val_;
            id = id_;
        }

        void Compute(MessageIterator msgs);

        int VertexID() {
            return id;
        }

        int Superstep() {
            return SUPERSTEP;
        }

        int NumVertices() {
            return NUM_VERTICES;
        }

        int NumNeightbors() {
            if (VERTEX_EDGES.find(id) == VERTEX_EDGES.end()) {
                vector<Edge> tmp;
                VERTEX_EDGES[id] = tmp;
            }
            return VERTEX_EDGES[id].size();            
        }

        double GetValue() {
            return value;
        }

        double Value() {
            return value;
        }

        void SetValue(double val) {
            value = val;
        }

        /**
         * Send the message to next round's local message buffer
         */
        void SendMessageTo(int dst_id, Message msg) {
            if (VERTEX_NEXT_MESSAGES.find(dst_id) == VERTEX_NEXT_MESSAGES.end()) {
                vector<Message> tmp;
                VERTEX_NEXT_MESSAGES[dst_id] = tmp;
            }
            VERTEX_NEXT_MESSAGES[dst_id].push_back(msg);
        }

        EdgeIterator GetOutEdges() {
            if (VERTEX_EDGES.find(id) == VERTEX_EDGES.end()) {
                vector<Edge> tmp;
                VERTEX_EDGES[id] = tmp;
            }
            return VERTEX_EDGES[id];
        }

        MessageIterator GetMessages() {
            if (VERTEX_IN_MESSAGES.find(id) == VERTEX_IN_MESSAGES.end()) {
                vector<Message> tmp;
                VERTEX_IN_MESSAGES[id] = tmp;
            }
            return VERTEX_IN_MESSAGES[id];
        }
        
        void VoteToHalt() {

        }
};
#endif