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
	mutex member_lock;
	mutex log_lock;
public:
	/* Init function */
	Daemon(int flag);
	
	/* First level functions */
 	void timeout(); // check whether its neighbors are down	
	void heartbeat(); // send hb to contacts
	void join(); // send join request to introducer, receive membership list
	void leave(); // send leave message to contacts
	void receive(); // listen to all messages endlessly
	void start(); // initial work. Join & start receive
	
	/* Membership functions */
	int updateMember(char * remote_ip, int flag);
	void updateContact(long long ts);
	void setMemberList(string s);

	/* Utility funcitons */
	long long unixTimestamp();
	void log(string s);
	void log(char *fmt, ...);

	/* Receiver handler functions */
	void joinHandler(char * remote_ip);
	
};
