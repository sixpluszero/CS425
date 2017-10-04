#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <map>
#include <ctime>
#include <string>
#include "socketlib.hpp"
#include "node.hpp"
#define NODE 10
#define INTRODUCER 1
using namespace std;

class Daemon{
private:
	vector<string> known_hosts;
	UDPSocket msg_socket;
	map<int, VMNode> member_list; // No. = index in the vector
	vector<VMNode> contact_list; // <IP, timestamp>
	int self_index;
	long long local_timestamp;
	//mutex lock;
public:
	Daemon(int flag);
	void updateContact();
 	void timeout(); // check whether its neighbors are down	
	void heartbeat(); // send hb to contacts
	void join(); // send join request to introducer, receive membership list, initialize contact list
	void leave(); // send leave message to contacts
	void receive(); // listen to all messages endlessly
	void updateMember();
	void start(); // initial work. Join & start receive
	void resolve(); // only for introducer. introduce a new member
	long long unix_timestamp();
	//string getSelfAddress();
};
