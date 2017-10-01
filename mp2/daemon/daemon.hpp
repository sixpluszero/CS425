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
	vector<Node> member_list; // No. = index in the vector
	map<string, long long> contact_list; // <IP, timestamp>
	int self_index;
	long long local_timestamp;
	mutex lock;
public:
	Daemon();
	updateContact(); 
	heartbeat(); // send hb to contacts
	join(); // send join request to introducer, receive membership list, initial contact list
	leave(); // send leave message to contacts
	receive(); // endless listen to all messages
	updateMember();
	start(); // initial work. Join & start receive
	resolve(); // Only for introducer. Introduce a new member
}
