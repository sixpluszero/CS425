#include "node.hpp"
using namespace std;

VMNode::VMNode(string _ip, long long _join_ts, int _id){
    ip = _ip;
    join_timestamp = _join_ts;
    id = _id;
}

/* 
 * Only used for VM1 for cold-start
 */
VMNode::VMNode(string _ip, long long _join_ts){
    ip = _ip;
    join_timestamp = _join_ts;
}

VMNode::VMNode(const VMNode &tmp) {
    ip = tmp.ip;
    id = tmp.id;
    join_timestamp = tmp.join_timestamp;
}

VMNode::VMNode(string s) {
    int idx;
    idx = s.find("/");
    id = stoi(s.substr(0, idx));
    s = s.substr(idx+1, s.length());
    idx = s.find("/");
    ip = s.substr(0, idx);
    s = s.substr(idx+1, s.length());
    join_timestamp = stoll(s);
}

VMNode::VMNode(){
}

string VMNode::toString() {
    return std::to_string(id) + "/" + ip + "/" + std::to_string(join_timestamp);
}
