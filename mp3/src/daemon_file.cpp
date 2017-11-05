#include "daemon.hpp"

/* send file through TCP socket */
void Daemon::sendFile(TCPSocket *sock, string fname) {
    plog("sending file from %s", fname.c_str());
    int TCPBUFSIZE = 4096;
    int bytesRead;
    FILE *fp = fopen(fname.c_str(), "rb");
    fseek(fp, 0L, SEEK_END);
    int sz = ftell(fp);
    plog("send: %s file size: %d bytes", fname.c_str(), sz);
    string lenStr = std::to_string(sz);
    int it = 10 - lenStr.length();
    for (int i = 0; i < it; i++) {
        lenStr = lenStr + " ";
    }
    sock->send(lenStr.c_str(), lenStr.length());
    fseek(fp, 0, 0);
    int total = 0;
    while (true) {
        char buffer[TCPBUFSIZE + 100];
        bytesRead = fread(buffer, 1, TCPBUFSIZE, fp);
        buffer[bytesRead] = '\0';
        total += bytesRead;
        sock->send(buffer, bytesRead);
        if (bytesRead < TCPBUFSIZE) break;
    }
    fclose(fp);
} 

void Daemon::recvFile(TCPSocket *sock, string fname) {
    plog("receiving file to %s", fname.c_str());
    int TCPBUFSIZE = 4096;
    int totalBytes = 1000000;
    int bytesReceived = 0;
    int totalBytesReceived = 0;
    FILE *fp = fopen(fname.c_str(), "wb");
    if (fp == NULL) plog("fp is null");
    while (totalBytesReceived < totalBytes) {
        char recvBuffer[TCPBUFSIZE + 100];
        if ((bytesReceived = (sock->recv(recvBuffer, TCPBUFSIZE))) <= 0) {
            cerr << "Unable to read";
            exit(1);
        }
        recvBuffer[bytesReceived] = '\0';
        if (totalBytesReceived == 0) {
            totalBytes = stoi(string(recvBuffer).substr(0, 10)) + 10;
            plog("file size is %d", totalBytes);
            fwrite(recvBuffer , sizeof(char), bytesReceived-10, fp);
            //fprintf(fp, "%s", recvBuffer+10);
        } else {
            fwrite(recvBuffer , sizeof(char), bytesReceived, fp);
            //fprintf(fp, "%s", recvBuffer);
        }
        totalBytesReceived += bytesReceived;
        //plog("debug %d/%d", totalBytesReceived, totalBytes);
    }
    plog("recv: %s file size: %d/%d bytes", fname.c_str(), totalBytesReceived, totalBytes);
    fclose(fp);
}

/* Response to the data saving request */
void Daemon::replicateFile(TCPSocket *sock, string input) {
    int dst_node = stoi(input.substr(0, input.find("/")));
    string fname = input.substr(input.find("/")+1, input.length());
    TCPSocket sock_w(member_list[dst_node].ip, BASEPORT+3);
    string ack;
    tcpSendString(&sock_w, "fileput;"+fname);
    ack = tcpRecvString(&sock_w);
    sendFile(&sock_w, "./mp3/files/"+fname);
    ack = tcpRecvString(&sock_w);
    if (ack == "success"){
        tcpSendString(sock, "success");
    } else {
        tcpSendString(sock, "fail");
    }
    
}
