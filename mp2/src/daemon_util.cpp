#include "daemon.hpp"

long long Daemon::unixTimestamp(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int now = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    return now;
}

void Daemon::log(string s, int flag){
    log_lock.lock();
    auto ts = unixTimestamp();
    double fts = (double)ts / 1000.0;
    FILE *fp = fopen(self_log.c_str(), "a");
    fprintf(fp, "[%02d-%.3f] %s\n", self_index, fts, s.c_str());
    fclose(fp);
    if (flag == 1) printf("[%02d-%.3f] %s\n", self_index, fts, s.c_str());
    log_lock.unlock();
}

/* Use with caution!! */
void Daemon::log(char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    log(string(buf), 0);
}

/* Use with caution!! */
void Daemon::plog(char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    log(string(buf), 1);
}

void Daemon::plog(string s) {
    log(s, 1);
}

void Daemon::setSelfAddr() {
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(ifa->ifa_name, "eth0") == 0) {
                self_ip = addressBuffer;
            }
        }
    }
    if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
}

void Daemon::setLogFile() {
    int vmid = stoi(self_ip.substr(self_ip.rfind('.') + 1, self_ip.length())) - 181;
    self_log = "server" + std::to_string(vmid) + ".log";
}