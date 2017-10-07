#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <map>
#include <ctime>
#include <string>
#include <sys/time.h>
#include <stdarg.h>
#include "socketlib.hpp"
#include "node.hpp"
#define NODE 10
#define INTRODUCER 3
#define HEARTBEAT 250000 /* (1/1000000 sec) Period for heartbeat() to wakeup and scan */
#define SCAN 500000 /* (1/1000000 sec) Period for timeout() to wakeup and scan */
#define FAILURE 1000 /* (1/1000 sec) Time for timeout() to detect the failure */
using namespace std;

class Daemon{
private:
	vector<string> known_hosts;
	UDPSocket msg_socket;
	UDPSocket cmd_socket; // Socket for command-line input
	map<int, VMNode> member_list;
	map<int, long long> contact_list;
	string self_ip;
	string self_log; // Location and filename of log in this vm;
	int self_index; // VM's index in the virtual ring
	bool leave_flag; // Receive flag for leaving the group and quit
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
	int newMember(char * remote_ip);
	void updateContact(long long ts);
	void setMemberList(string s);
	string contactsToString();
	string membersToString();

	/* Utility funcitons */
	long long unixTimestamp();
	void log(string s, int flag = 0);
	void log(char *fmt, ...);
	void plog(string s);
	void plog(char *fmt, ...);
	void setSelfAddr();
	void setLogFile();
	void timed_recv(char *rip, char *buf, bool *flag);

	/* Receiver handler functions */
	void joinHandler(char *remote_ip);
	void updateHandler(string msg);
	void heartbeatHandler(char *remote_ip);
};
