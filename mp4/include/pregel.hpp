#include <stdio.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <assert.h>
using namespace std;

class Message {
    private:
        double value;
    public:
        double Value() {
            return value;
        }
        Message(double val) {
            value = val;
        }
};

class Edge {
    private:
        int src;
        int dst;
        double val;
    public:
        int Src() {
            return src;
        }
        int Dst() {
            return dst;
        }
        double Value() {
            return val;
        }
        Edge(int s, int t, double v) {
            src = s;
            dst = t;
            val = v;
        }
};