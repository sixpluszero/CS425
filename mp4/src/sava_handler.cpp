#include "daemon.hpp"

void Daemon::savaHandler(TCPSocket *sock) {
    string info, cmd;
    int r;
    r = sock->recvStr(info);
    if (r == 1) {
        plog("Error in client. Close the connection.");
        delete sock;
        return;
    }
    plog("channel recv: %s", info.c_str());
    try {
        if (prefixMatch(info, "savanewtask")) {
            string app, input, output, comb;
            info = info.substr(12, info.length());
            app = info.substr(0, info.find(";"));
            info = info.substr(app.length()+1, info.length());
            input = info.substr(0, info.find(";"));
            info = info.substr(input.length()+1, info.length());
            if (info.find(";") == -1) {
                output = info;
            } else {
                output = info.substr(0, info.find(";"));
                info = info.substr(output.length()+1, info.length());
                comb = info;    
            }
            if (!hasFile(input)) {
                sock->sendStr("error: input file not found.");
                return;
            }
            if (hasFile(output)) {
                sock->sendStr("error: output file already exists.");
                return;
            }
            sock->sendStr("ack");
            savaTask(sock, app, input, output, comb);

        } else if (prefixMatch(info, "savasendinputs")){
            sock->sendStr("ack");
            sock->recvFile("vertices.txt");
            sock->sendStr("ack");
            sock->recvFile("edges.txt");
            sock->sendStr("ack");
            sock->recvFile("runner");
            sock->sendStr("ack");
            cmd = "cp ./mp4/tmp/vertices.txt ./mp4/sava/vertices.txt";
            system(cmd.c_str());
            cmd = "cp ./mp4/tmp/edges.txt ./mp4/sava/edges.txt";
            system(cmd.c_str());
            cmd = "cp ./mp4/tmp/runner ./mp4/sava/runner";
            system(cmd.c_str());
        } else {
            return;
        }
    } catch (...) {
        plog("Error in SAVA execution");
        return;
    }
}