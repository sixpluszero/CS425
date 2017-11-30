#include "daemon.hpp"

int Daemon::putFile(TCPSocket *sock, string fname) {
  if (sock->sendStr("ack")) return 1;
  if (sock->recvFile(fname)) return 1;
  if (sock->sendStr("success")) return 1;
  return 0;
}

/* Response to the data saving request */
void Daemon::replicateFile(TCPSocket *sock, string input) {
  int dst_node = stoi(input.substr(0, input.find("/")));
  string fname = input.substr(input.find("/")+1, input.length());
  TCPSocket sock_w(member_list[dst_node].ip, BASEPORT+3);
  string ack;
  if (sock_w.sendStr("fileput;"+fname) == 1) {
    sock->sendStr("fail");
    return;
  }
  if (sock_w.recvStr(ack) == 1) {
    sock->sendStr("fail");
    return;
  }   
  if (sock_w.sendFile("./mp4/files/"+fname) == 1) {
    sock->sendStr("fail");
    return;
  }
  if (sock_w.recvStr(ack) == 1) {
    sock->sendStr("fail");
    return;
  }
  sock->sendStr("success");
  return;
}

/**
 *  fixReplication() starts re-replication issued by master 
 */
void Daemon::fixReplication(){
  return;
  plog("start fixing replication");
  for (auto it = file_location.begin(); it != file_location.end(); it++) {
    if (it->second.size() == REPLICA) continue;
    while (it->second.size() < REPLICA && member_list.size() >= REPLICA) { /* If data node is less than four, don't fix. */
      /* Randomly pick one node that has this file */
      vector<int> cand;
      for (auto it_0 = it->second.begin(); it_0 != it->second.end(); it_0++) {
        cand.push_back(it_0->first);
      }
      std::random_shuffle(cand.begin(), cand.end());
      int src_node = cand[0];
      plog("replica src node is %d", src_node);
      string fname = it->first;
      plog("fixing %s using replica at %d", fname.c_str(), src_node);
      while (it->second.size() < REPLICA && member_list.size() >= REPLICA) {
        /* Randomly pick a new data node */
        cand.clear();
        for (auto it_0 = member_list.begin(); it_0 != member_list.end(); it_0++) {
          cand.push_back(it_0->first);
        }
        std::random_shuffle(cand.begin(), cand.end());
        for (auto it_0 = cand.begin(); it_0 != cand.end(); it_0++) {
          if (it->second.find(*it_0) == it->second.end()) {
            // This is a data slot to fit in
            int dst_node = *it_0;
            plog("replica dst node is %d", dst_node);
            plog("fixing %s picking %d", fname.c_str(), dst_node);
            try {
              TCPSocket sock_(member_list[src_node].ip, BASEPORT+3);
              string ack;
              int r;
              r = sock_.sendStr("copy;"+std::to_string(dst_node)+"/"+fname);
              if (r == 1) {
                continue;
              }
              r = sock_.recvStr(ack);
              if (r == 1) {
                continue;
              }
              plog("node %d reply with %s", src_node, ack.c_str());
              if (ack == "success"){
                plog("add replication of %s from %d to %d", fname.c_str(), src_node, dst_node);
                string update = fname + "/" + std::to_string(dst_node) + "/" + std::to_string(unixTimestamp());
                newFileMappingLocation(update);
                for (auto it2 = master_list.begin(); it2 != master_list.end(); it2++) {
                  if (it2->second == "Primary") continue;
                  TCPSocket sock_m(member_list[it2->first].ip, BASEPORT+3);
                  int r = sock_m.sendStr("newfloc;"+update);
                  if (r == 1) {
                    continue;
                  }
                }
                break;
              } else {
                plog("Error in replicating(dst).");    
                continue;
              }
            } catch (...) {
              plog("Error in replicating(src).");
              continue;
            }
          }
        }
      }
    }
  }   
  plog(fileMappingToString().c_str());
  plog("end fixing replication");
}

/* sdfsHandler() handles SDFS port request */
void Daemon::sdfsHandler(TCPSocket *sock) {
  string info;
  int r;
  r = sock->recvStr(info);
  if (r == 1) {
    plog("Unknown error in sender side. Close the connection.");
    delete sock;
    return;
  }
  plog("channel recv: %s", info.c_str());
  try {
    if (prefixMatch(info, "newfmap")) { /* Backup master receive full file_location */
      newFileMapping(info.substr(8, info.length()));
      plog("synced file mapping: %s", fileMappingToString().c_str());
      sock->sendStr("ack");
    } else if (prefixMatch(info, "newfloc")) { /* Backup master receive new file_location entry */
      newFileMappingLocation(info.substr(8, info.length()));
      plog("updated file mapping: %s", fileMappingToString().c_str());
    } else if (prefixMatch(info, "fileput")) {
      string fname = "./mp4/files/" + info.substr(8, info.length());
      putFile(sock, fname);
    } else if (prefixMatch(info, "fileget")) { /* Datanode send file to master */
      string fname = "./mp4/files/" + info.substr(8, info.length());
      sock->sendFile(fname);
    } else if (prefixMatch(info, "copy")) { 
      replicateFile(sock, info.substr(5, info.length()));
    } else if (prefixMatch(info, "masterfiledel")) { /* Backup master receive delete file location request */
      file_location.erase(info.substr(14, info.length()));
    } else if (prefixMatch(info, "filedel")) { /* Datanode receive delete file request from master */ 
      string cmd = "rm ./mp4/files/"+info.substr(8, info.length());
      system(cmd.c_str());
    } else if (prefixMatch(info, "clientlist")) { /* Primary master receive LIST command from client */
      clientList(sock, info.substr(11, info.length()));
    } else if (prefixMatch(info, "clientput")) { /* Primary master receive PUT command from client */
      clientPut(sock, info.substr(10, info.length()));
    } else if (prefixMatch(info, "clientget")) { /* Primary master receive GET command from client */
      clientGet(sock, info.substr(10, info.length()));
    } else if (prefixMatch(info, "clientdel")) { /* Primary master receive DELETE command from client */
      clientDel(sock, info.substr(10, info.length()));
    }    
  } catch (...) {
    plog("encounter error at node msg channel with request %s", info.c_str());
    delete sock;
    return;
  }
  delete sock;
}

void Daemon::sdfs() {
  while (leave_flag == false) {
    for (;;) {
      TCPSocket *sock = node_socket.accept();
      std::thread msg_t(&Daemon::sdfsHandler, this, sock);
      msg_t.detach();
    }
  }
  plog("module channel exit");
}
