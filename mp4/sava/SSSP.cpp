#include "exec.hpp"
#define DOUBLEINF 10000000.0

void Vertex::Compute(MessageIterator msgs) {
    if (Superstep() == 0) {
        if (VertexID() == 1) {
            SetValue(0.0);
        } else {
            SetValue(DOUBLEINF);
        }
    } else {
        double minv = GetValue();
        for (auto x : GetMessages()) {
            minv = min(minv, x.Value());
        }
        SetValue(minv);
    }
    double minv = GetValue();
    for (auto x : GetOutEdges()) {
        SendMessageTo(x.Dst(), minv + x.Value());
    }
}
