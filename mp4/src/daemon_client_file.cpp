#include "daemon.hpp"

/* Master: Client put */
void Daemon::clientPut(TCPSocket *sock, string fname) {
  if (role != "Primary"){
    sock->sendStr("rej");
    return;
  }
  if (hasFile(fname)) { /* We are going to do an update */
    long long ots = fileLatestTime(fname);
    long long nts = unixTimestamp();
    if ((nts - ots) < 60 * 1000) {
      string cmd;
      sock->sendStr("has old replica in last minute");
      sock->recvStr(cmd);
      if (cmd == "No"){
        plog("User reject file update");
        return;
      } else {
        plog("User accept file update");
        sock->sendStr("ack");
      }
    } else {
      sock->sendStr("ack");
    }
  } else {
    sock->sendStr("ack");
  }
  string tmp_file = "./mp4/tmp/"+fname;
  sock->recvFile(tmp_file);
  plog("file received at temp file %s", tmp_file.c_str());

  vector<int> cand;
  if (hasFile(fname)) {
    for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
      cand.push_back(it->first);
    }
  } else {
    for (auto it = member_list.begin(); it != member_list.end(); it++) {
      cand.push_back(it->first);
    }    
  }
  std::random_shuffle(cand.begin(), cand.end());

  int cnt = 0;
  for (auto it = cand.begin(); it != cand.end(); it++) {
    int nid = *it; 
    string ack;
    int r;
    TCPSocket sock_w(member_list[nid].ip, BASEPORT+3);
    r = sock_w.sendStr("fileput;"+fname);
    if (r == 1) {
      plog("Error in replicating new file to %d", nid);
      continue;
    }
    r = sock_w.recvStr(ack);
    if (r == 1) {
      plog("Error in replicating new file to %d", nid);
      continue;
    }
    r = sock_w.sendFile(tmp_file);
    if (r == 1) {
      plog("Error in replicating new file to %d", nid);
      continue;
    }
    r = sock_w.recvStr(ack);
    if (r == 1) {
      plog("Error in replicating new file to %d", nid);
      continue;
    }
    plog("response from replica server: %s", ack.c_str());
    if (ack == "success") {
      string update = fname + "/" + std::to_string(nid) + "/" + std::to_string(unixTimestamp());
      newFileMappingLocation(update);
      /* Send */
      for (auto it = master_list.begin(); it != master_list.end(); it++) {
        if (it->second == "Primary") continue;
        TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
        sock_.sendStr("newfloc;"+update);
      }
      cnt++;
      if (cnt == 3) {
        plog("reach quorum");
        sock->sendStr("success");
      }
      if (cnt == 4) {
        break;
      }    
    } else {
      plog("Error in replicating new file to %d", nid);
      continue;
    }
  }
  plog("updated file mapping: %s", fileMappingToString().c_str());
}

void Daemon::clientGet(TCPSocket *sock, string fname){
  string ack;
  if (role != "Primary"){
    sock->sendStr("rej");
    return;
  }
  if (hasFile(fname)) { 
    sock->sendStr("ack");
    sock->recvStr(ack);
    for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
      int r;
      TCPSocket sock_w(member_list[it->first].ip, BASEPORT+3);
      r = sock_w.sendStr("fileget;"+fname);
      if (r == 1) continue;
      r = sock_w.recvFile("./mp4/tmp/"+fname);
      if (r == 1) continue;
      r = sock->sendFile("./mp4/tmp/"+fname);
      if (r == 1) continue;
      break;
    }
    return;
  } else {
      sock->sendStr("file not exists");
      return;
  }

}

void Daemon::clientDel(TCPSocket *sock, string fname){
  string ack;
  if (role != "Primary"){
    sock->sendStr("rej");
    return;
  }
  if (hasFile(fname)) {
    for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
      TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
      sock_.sendStr("filedel;"+fname);
    }
    file_location.erase(fname);
    for (auto it = master_list.begin(); it != master_list.end(); it++) {
      if (it->second == "Primary") continue;
      TCPSocket sock_(member_list[it->first].ip, BASEPORT+3);
      sock_.sendStr("masterfiledel;"+fname);
    }
    sock->sendStr("success");
    return;
  } else {
      sock->sendStr("file not exists");
      return;        
  }
}

void Daemon::clientList(TCPSocket *sock, string fname){
  string ack;
  if (role != "Primary"){
    sock->sendStr("rej");
    return;
  }
  if (hasFile(fname)) {
    string result;
    for (auto it = file_location[fname].begin(); it != file_location[fname].end(); it++) {
      if (result != "") result += ",";
      result += member_list[it->first].ip;
    }
    result = "(" + result + ")";
    sock->sendStr(result);
    return;
  } else {
    sock->sendStr("file not exists");
    return;
  }
}
