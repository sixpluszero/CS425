#include "daemon.hpp"

void Daemon::savaHandler(TCPSocket *sock) {
    string info, cmd, fname;
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
            plog("New Task: %s %s %s %s", app.c_str(), input.c_str(), output.c_str(), comb.c_str());
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
            SAVA_WORKER_CONN.clear();
            SAVA_WORKER_MAPPING.clear();
            plog("Client state: %lu, %lu, %lu, %lu", SAVA_VERTEX_MAPPING.size(), SAVA_WORKER_MAPPING.size(), SAVA_EDGES.size(), SAVA_WORKER_CONN.size());
            SAVA_STATE = 0;
            SAVA_NUM_WORKER = 0;
            SAVA_NUM_VERTICES = 0;
            SAVA_WORKER_ID = 0;
            SAVA_NUM_MSG = 0;
            plog("Client state: %d %d %d %d %d", SAVA_STATE, SAVA_NUM_WORKER, SAVA_NUM_VERTICES, SAVA_WORKER_ID, SAVA_NUM_MSG);
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
            int rw = stoi(info.substr(0, info.find(";")));
            info = info.substr(info.find(";")+1, info.length());
            int rr = stoi(info.substr(0, info.find(";")));
            plog("%d: %d. (%d)", rw, rr, SAVA_ROUND);
            while (rr != SAVA_ROUND);
            plog("info from %d", rw);
            if (sock->sendStr("ack")) {
                plog("error in sending ack");
            }
            fname = "./mp4/sava/rmsgs_" + to_string(rw) + ".txt";
            plog("info ok %d %s", rw, fname.c_str());
            if (sock->recvFile(fname)) {
                plog("error in receiving remote messages from %d", rw);
            } else {
                printf("file received ok\n");
            }
            printf("file ok\n");
            plog("info %d want to inc msg cnt", rw);
            msg_lock.lock();
            SAVA_NUM_MSG++;
            msg_lock.unlock();
            plog("finish this message's collection");
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