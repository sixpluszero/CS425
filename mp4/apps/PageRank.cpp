#include "exec.hpp"
#define DOUBLEINF 10000000.0

void Vertex::Compute(MessageIterator msgs) {
    if (Superstep() == 0) SetValue(1.0 / NumVertices());
    if (Superstep() >= 1) {
        double sumv = 0;
        for (auto x : GetMessages()) {
            sumv += x.Value();
        }
        SetValue(0.15 / NumVertices() + 0.85 * sumv);
    }

    if (Superstep() < 20) {
        int n = NumNeightbors();
        for (auto x : GetOutEdges()) {
            SendMessageTo(x.Dst(), GetValue() / n);
        }
    }
}
