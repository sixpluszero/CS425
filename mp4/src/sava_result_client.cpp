#include "daemon.hpp"

void Daemon::savaClientResult(TCPSocket *sock, int type, int num) {
    string ack, cmd;
    RUNNER_SOCK = new TCPSocket(member_list[SAVA_WORKER_MAPPING[SAVA_WORKER_ID]].ip, 9999);
    cmd = "result;" + to_string(type) + ";" + to_string(num) + ";";
    if (RUNNER_SOCK->sendStr(cmd)) plog("Error in sending cmd to runner");
    if (RUNNER_SOCK->recvStr(ack)) plog("Error in receiving ack from runner");
    if (sock->sendFile("./mp4/tmp/localres")) plog("Error in sending partial result to master");
    system("pkill runner");
    plog("runner killed");
    system("rm ./mp4/tmp/*");
    plog("tmp folder cleared");
    system("rm ./mp4/sava/*.txt");
    plog("sava folder text cleared");
}
