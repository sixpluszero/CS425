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
            string app, input, output, comb, cmd;
            info = info.substr(12, info.length());
            app = info.substr(0, info.find(";"));
            info = info.substr(app.length()+1, info.length());
            input = info.substr(0, info.find(";"));
            info = info.substr(input.length()+1, info.length());
            if (info.find(";") == std::string::npos) {
                output = info;
            } else {
                output = info.substr(0, info.find(";"));
                info = info.substr(output.length()+1, info.length());
                comb = info;    
            }
            plog("New Task: %s %s %s %s %s", app.c_str(), input.c_str(), output.c_str(), comb.c_str());
            if (!hasFile(input)) {
                sock->sendStr("error: input file not found.");
                return;
            }
            if (hasFile(output)) {
                sock->sendStr("error: output file already exists.");
                return;
            }
            sock->sendStr("ack");

            if (savaComplieApp(sock)) {
                sock->sendStr("error: Client code cannot be compiled.");
                return;
            }
    
            while (savaTask(sock, app, input, output, comb)) {
            };
        } else if (prefixMatch(info, "savasendinput")){
            string msg;
            system("pkill runner");
            system("rm ./mp4/sava/*.txt");
            system("rm ./mp4/sava/runner");
            sock->sendStr("ack");
            sock->recvFile("./mp4/sava/vertices.txt");
            plog("vtx file received");
            sock->sendStr("ack");
            sock->recvFile("./mp4/sava/edges.txt");
            plog("edges file received");
            sock->sendStr("ack");
            sock->recvFile("./mp4/sava/runner");
            plog("binary file received");
            system("chmod 777 ./mp4/sava/runner");
            sock->sendStr("ack");
            plog("files received");
            sock->recvStr(msg);
            sock->sendStr("ack");
            SAVA_APP_NAME = msg.substr(0, msg.find(";"));
            SAVA_COMBINATOR = msg.substr(msg.find(";")+1, msg.length());
            plog("APP: %s COMBINATOR: %s", SAVA_APP_NAME.c_str(), SAVA_COMBINATOR.c_str());
        } else if (prefixMatch(info, "savaclientinit")) {
            SAVA_NUM_VERTICES = stoi(info.substr(info.find(";")+1, info.length()));
            sock->sendStr("ack");
            sock->recvFile("./mp4/sava/fullwmap.txt");
            sock->sendStr("ack");
            sock->recvFile("./mp4/sava/fullvmap.txt");
            savaInitPregelClient();
            sock->sendStr("ack");
        } else if (prefixMatch(info, "savaclientstep")) {
            int step;
            step = stoi(info.substr(info.find(";")+1, info.length()));
            savaClientSuperstep(sock, step);            
        } else if (prefixMatch(info, "savaclientresult")) {
            info = info.substr(info.find(";")+1, info.length());
            string type = info.substr(0, info.find(";"));
            info = info.substr(info.find(";")+1, info.length());
            if (type == "min") {
                savaClientResult(sock, 1, stoi(info));
            } else if (type == "max") {
                savaClientResult(sock, 2, stoi(info));
            } else {
                savaClientResult(sock, 0, 0);
            }
        } else if (prefixMatch(info, "savaworkermsgs")) {
            info = info.substr(info.find(";")+1, info.length());
            sock->sendStr("ack");
            string fname = "./mp4/sava/rmsgs_" + info.substr(0, 1) + ".txt";
            sock->recvFile(fname);
            msg_lock.lock();
            SAVA_NUM_MSG++;
            msg_lock.unlock();
        } else if (prefixMatch(info, "savareplicatemeta")) {
            info = info.substr(info.find(";")+1, info.length());
            SAVA_APP_NAME = info.substr(0, info.find(";"));
            info = info.substr(info.find(";")+1, info.length());
            SAVA_INPUT = info.substr(0, info.find(";"));
            info = info.substr(info.find(";")+1, info.length());
            SAVA_OUTPUT = info.substr(0, info.find(";"));
            info = info.substr(info.find(";")+1, info.length());
            SAVA_COMBINATOR = info.substr(0, info.find(";"));
            info = info.substr(info.find(";")+1, info.length());
            SAVA_STATE = stoi(info.substr(0, info.find(";")));
            info = info.substr(info.find(";")+1, info.length());
            SAVA_ROUND = stoi(info.substr(0, info.find(";")));
            sock->sendStr("ack");
            system("./mp4/sava/runner");
            if (sock->recvFile("./mp4/sava/runner")){
                plog("Error in receiving binary file");
                return;
            } else {
                sock->sendStr("ack");
            }


        } else {
            sock->sendStr("ack");
            return;
        }
    } catch (...) {
        plog("Error in SAVA execution");
        return;
    }
}