#include <mutex>
#include <threading>
#include <algorithm>
#include <vector>
#include <map>
#include "../udp/socketlib.hpp"
#include "../node/node.hpp"

class Daemon(){
private:
	UDPSocket sock_service;
    vector<Node> member_list;
    map<string, long long> contact_list;
    int self_index;
    long long local_timestamp;
    mutex lock;
public:
	Daemon();
    updateContact(); 
	heartbeat();
    join();
	leave();
	receive();
	updateMember();
    resolve(); /* Only for introducer */
    start();
}

