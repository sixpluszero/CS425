#include "daemon.hpp"

void Daemon::savaClientResult(TCPSocket *sock, int type, int num) {
    priority_queue< KV, vector<KV>, greater<KV> > minHeap;
    priority_queue< KV > maxHeap;
    FILE *fp;
    int cnt = 0;
    if (type == 0) {
        fp = fopen("./mp4/tmp/localres", "w");
        for (auto x : PREGEL_LOCAL_VERTICES) {
            fprintf(fp, "%d %lf\n", x.first, x.second);
        }
        fclose(fp);
        sock->sendFile("./mp4/tmp/localres");
        return;
    }

    if (type == 1) {
        plog("DEBUG MIN!!!!");
        for (auto x : PREGEL_LOCAL_VERTICES) {
            minHeap.push(KV(x.first, x.second));
        }
        fp = fopen("./mp4/tmp/localres", "w");
        while (!minHeap.empty()) {
            fprintf(fp, "%d %lf\n", minHeap.top().id, minHeap.top().value);
            cnt++;
            if (cnt == num) break;
            minHeap.pop();
        }
        fclose(fp);
        sock->sendFile("./mp4/tmp/localres");
        return;
    }

    if (type == 2) {
        plog("DEBUG MAX!!!!");
        for (auto x : PREGEL_LOCAL_VERTICES) {
            maxHeap.push(KV(x.first, x.second));
        }
        fp = fopen("./mp4/tmp/localres", "w");
        while (!maxHeap.empty()) {
            fprintf(fp, "%d %lf\n", maxHeap.top().id, maxHeap.top().value);
            cnt++;
            if (cnt == num) break;
            maxHeap.pop();
        }
        fclose(fp);
        sock->sendFile("./mp4/tmp/localres");
        return;
    }

}