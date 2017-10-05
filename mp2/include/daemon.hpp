#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <map>
#include <ctime>
#include <string>
#include <sys/time.h>
#include "socketlib.hpp"
#include "node.hpp"
#define NODE 10
#define INTRODUCER 1
using namespace std;

class Daemon{
private:
	vector<string> known_hosts;
	UDPSocket msg_socket;
	map<int, VMNode> member_list;
	map<int, long long> contact_list;
	int self_index;
	long long local_timestamp;
	//mutex lock;
public:
	Daemon(int flag);
	
 	void timeout(); // check whether its neighbors are down	
	void heartbeat(); // send hb to contacts
	void join(); // send join request to introducer, receive membership list, initialize contact list
	void leave(); // send leave message to contacts
	void receive(); // listen to all messages endlessly
	void start(); // initial work. Join & start receive
	
	void joinHandler(char * remote_ip);
	int updateMember(char * remote_ip, int flag);
	void updateContact(long long ts);
	void setMemberList(string s);

	//void resolve(); // only for introducer. introduce a new member
	long long unix_timestamp();
	//string getSelfAddress();
};
