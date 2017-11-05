#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <map>
#include <ctime>
#include <string>
#include <sys/time.h>
#include <iostream>
#include <stdarg.h>
#include "socketlib.hpp"
#include "tcpsocket.hpp"
#include "node.hpp"
#include "config.hpp"
#include "fstream"
#define NODE 10
#define DROPRATE 0
#define INTRODUCER 10
#define HEARTBEAT 200000 /* (1/1000000 sec) Period for heartbeat() to wakeup and scan */
#define SCAN 1000000 /* (1/1000000 sec) Period for timeout() to wakeup and scan */
#define FAILURE 1500 /* (1/1000 sec) Time for timeout() to detect the failure */

using namespace std;

class Daemon{
private:
	vector<string> known_hosts;
	UDPSocket out_socket; // For sending message to client ONLY
	UDPSocket msg_socket; // Internal membership message
	UDPSocket cmd_socket; // For client commands
	TCPServerSocket node_socket; // For file system realted communication
	map<int, VMNode> member_list;
	map<int, long long> contact_list;
	string self_ip;
	string self_log; // Location and filename of log in this vm;
	int self_index; // VM's index in the virtual ring
	bool leave_flag; // Receive flag for leaving the group and quit
	long long local_timestamp;
	mutex member_lock;
	mutex log_lock;
	string role;
	map<int, string> master_list;
	map<string, map<int, long long> > file_location;
public:
	/* Init function */
	Daemon(int flag);
	
	/* First level functions */
 	void timeout(); // check whether its neighbors are down	
	void heartbeat(); // send hb to contacts
	void join(); // send join request to introducer, receive membership list
	void command(); // send leave message to contacts
	void receive(); // listen to all membership related messages
	void channel(); // handle master events
	
	/* Master functions */
	bool isPrimary();
	bool isBackup();
	bool isMaster();
	bool hasPrimary();
	bool isFirstBackup();
	void assignBackup(int num);
	void upgradeBackup();
	void fixReplication();

	/* File functions */
	void clearNodeFile(int id);
	void initFileMapping();
	string fileMappingToString();
	void newFileMapping(string input);
	void newFileMappingLocation(string input);
	bool hasFile(string fname);
	int replicaCount(string fname);
	long long fileLatestTime(string fanme);
	void replicateFile(TCPSocket *sock, string input);
	void sendFile(TCPSocket *sock, string fname);
	void recvFile(TCPSocket *sock, string fname);

	void clientPut(TCPSocket *sock, string fname);
	void clientGet(TCPSocket *sock, string fname);
	void clientDel(TCPSocket *sock, string fname);
	void clientList(TCPSocket *sock, string fname);


	/* Membership functions */
	int newMember(char * remote_ip);
	void updateContact(long long ts);
	void setMemberList(string s);
	void setMasterList(string s);
	string contactsToString();
	string membersToString();
	string mastersToString();

	/* Utility funcitons */
	long long unixTimestamp();
	void log(string s, int flag = 0);
	void log(const char *fmt, ...);
	void plog(string s);
	void plog(const char *fmt, ...);
	void setSelfAddr();
	void setLogFile();
	bool dropMsg();
	void tcpSendString(TCPSocket *sock, string in);
	string tcpRecvString(TCPSocket *sock);
	bool prefixMatch(string org, string patt);

	/* Receiver handler functions */
	void joinHandler(char *remote_ip);
	void updateHandler(string msg);
	void heartbeatHandler(char *remote_ip);
	void nodeMsgHandler(TCPSocket *sock);
};
