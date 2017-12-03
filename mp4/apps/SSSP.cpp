#include "exec.hpp"
#define DOUBLEINF 10000000.0

void Vertex::Compute() {
    if (Superstep() == 0) SetValue(DOUBLEINF);
    double minDist = (VertexID() == 1) ? 0 : DOUBLEINF;
    for (auto x : GetMessages()) {
        minDist = min(minDist, x.Value());
    }
    if (minDist < GetValue()) {
        SetValue(minDist);
        for (auto x : GetOutEdges()) {
            SendMessageTo(x.Dst(), minDist + x.Value());
        }
    }
}
