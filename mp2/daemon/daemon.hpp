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
	map<int, Node> member_list; // No. = index in the vector
	vector<Node> contact_list; // <IP, timestamp>
	int self_index;
	long long local_timestamp;
	mutex lock;
public:
	Daemon();
	updateContact();
 	timeout(); // check whether its neighbors are down	
	heartbeat(); // send hb to contacts
	join(); // send join request to introducer, receive membership list, initial contact list
	leave(); // send leave message to contacts
	receive(); // endless listen to all messages
	updateMember();
	start(); // initial work. Join & start receive
	resolve(); // only for introducer. introduce a new member
}
