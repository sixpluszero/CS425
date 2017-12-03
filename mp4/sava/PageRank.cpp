#include "exec.hpp"
#define DOUBLEINF 10000000.0
double ALPHA = 0.15;
void Vertex::Compute() {
    //if (Superstep() == 0) SetValue(1.0 / NumVertices());
    if (Superstep() == 0) SetValue(1.0);
    if (Superstep() >= 1) {
        double sumv = 0;
        for (auto x : GetMessages()) {
            sumv += x.Value();
        }
        //SetValue(ALPHA / NumVertices() + (1.0 - ALPHA) * sumv);
        SetValue(ALPHA + (1.0 - ALPHA) * sumv);
    }

    if (Superstep() < 20) {
        int n = NumNeightbors();
        for (auto x : GetOutEdges()) {
            SendMessageTo(x.Dst(), GetValue() / n);
        }
    }
}
