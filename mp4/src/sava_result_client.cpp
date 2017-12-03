#include "daemon.hpp"

void Daemon::savaClientResult(TCPSocket *sock, int type, int num) {
    string ack, cmd;
    TCPSocket * exec_sock = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    cmd = "result;" + to_string(type) + ";" + to_string(num) + ";";
    exec_sock->sendStr(cmd);
    exec_sock->recvStr(ack);
    sock->sendFile("./mp4/tmp/localres");
}
