#include "daemon.hpp"

long long Daemon::unixTimestamp(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int now = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return now;
}

void Daemon::log(string s){
    log_lock.lock();
    auto ts = unixTimestamp();
    printf("[%lld] %s\n", ts, s.c_str());
    FILE *fp = fopen("./server.log", "w+");
    fprintf(fp, "[%lld] %s\n", ts, s.c_str());
    fclose(fp);
    log_lock.unlock();
}