#include <mutex>
#include <thread>
#include <algorithm>
#include <vector>
#include <queue>
#include <map>
#include <ctime>
#include <string>
#include <sys/time.h>
#include <iostream>
#include <stdarg.h>
#include "udpsocket.hpp"
#include "tcpsocket.hpp"
#include "node.hpp"
#include "config.hpp"
#include "pregel.hpp"
#include "signal.h"
#include <fstream>
#include <chrono>
#define NODE 10
#define DROPRATE 0
#define INTRODUCER 10
#define HEARTBEAT 200000 	// (1/1000000 sec) Period for heartbeat() to wakeup and scan
#define SCAN 1000000 		// (1/1000000 sec) Period for timeout() to wakeup and scan
#define FAILURE 2000 		// (1/1000 sec) Time for timeout() to detect the failure
#define NUMMASTER 2
#define REPLICA 3
using namespace std;

class Daemon{
private:
	vector<string> known_hosts;
	UDPSocket out_socket; // For sending message to client ONLY
	UDPSocket msg_socket; // Internal membership message
	UDPSocket cmd_socket; // For client commands
	TCPServerSocket node_socket; // For file system realted communication
	TCPServerSocket sava_socket; // For SAVA framework communication
	map<int, VMNode> member_list;
	map<int, long long> contact_list;
	string self_ip;
	string self_log; // Location and filename of log in this vm;
	int self_index; // VM's index in the virtual ring
	bool leave_flag; // Receive flag for leaving the group and quit
	long long local_timestamp;
	mutex member_lock;
	mutex log_lock;
	mutex msg_lock; // Log for SAVA worker inter communication.
	string role;
	map<int, string> master_list;
	map<string, map<int, long long> > file_location;
	/**
	 * Below is data structure for MP4 SAVA
	 */ 
	string SAVA_APP_NAME;
	string SAVA_INPUT;
	string SAVA_OUTPUT;
	string SAVA_COMBINATOR;
	int SAVA_GRAPH;
	int SAVA_ROUND;
	int SAVA_STATE; // 0: Init, 1: Running, 2: Stop
	int SAVA_NUM_WORKER;
	int SAVA_NUM_VERTICES;
	int SAVA_WORKER_ID;
	int SAVA_NUM_MSG;
	map<int, int> SAVA_VERTEX_MAPPING;
	map<int, int> SAVA_WORKER_MAPPING;
	map<int, vector<int>> SAVA_EDGES;
	map<int, TCPSocket* > SAVA_WORKER_CONN;
	map<int, vector<double>> SAVA_REMOTE_MSGS;

	map<int, double> PREGEL_LOCAL_VERTICES;
	map<int, vector<Edge>> PREGEL_LOCAL_EDGES;
	map<int, vector<Message>> PREGEL_IN_MESSAGES, PREGEL_OUT_MESSAGES;
	

public:
	/* Init function */
	Daemon(int flag);
	
	/* First level functions */
 	void 	timeout(); // check whether its neighbors are down	
	void 	heartbeat(); // send hb to contacts
	void 	join(); // send join request to introducer, receive membership list
	void 	command(); // send leave message to contacts
	void 	receive(); // listen to all membership related messages
	void 	sdfs(); // handle file related events
	
	/* Master functions */
	bool 	isPrimary();
	bool 	isBackup();
	bool 	isMaster();
	bool 	hasPrimary();
	bool 	isFirstBackup();
	void 	assignBackup(int num);
	void 	upgradeBackup();

	/* Sava functions */
	void  	sava();
	int  	savaComplieApp(TCPSocket *sock);
	int  	savaCompile();
	int  	savaReplicateMeta();
	int	 	savaPartitionGraph();
	void 	savaInitPregelMaster();
	void 	savaInitPregelClient();
	int  	savaMasterSuperstep();
	void 	savaMasterSuperstepThread(int wid);
	string 	savaMasterGetTopResult(int topN, int recv);
	int  	savaClientSuperstep(TCPSocket *sock, int step);
	void 	savaClientResult(TCPSocket *sock, int type, int num);
	void 	savaHandler(TCPSocket *sock);
	int 	savaTask(TCPSocket *sock, string app, string input, string output, string comb);
	void 	pregelInitStep();
    void 	pregelWriteEdges();
    void 	pregelWriteMessages();
    void 	pregelWriteVertices();
    void 	pregelExecution();
    void 	pregelReadVertices();
    void 	pregelReadLocalMessages();
	void 	pregelCombineMessages();
    void 	pregelGenRemoteMessages();
    void 	pregelReadRemoteMessages();

	/* File functions */
	void 		clearNodeFile(int id);
	void 		initFileMapping();
	string 		fileMappingToString();
	void 		newFileMapping(string input);
	void 		newFileMappingLocation(string input);
	bool 		hasFile(string fname);
	int 		replicaCount(string fname);
	long long 	fileLatestTime(string fanme);

	void 		fixReplication();
	void 		replicateFile(TCPSocket *sock, string input);
	int 		putFile(TCPSocket *sock, string fname);

	void clientPut(TCPSocket *sock, string fname);
	void clientGet(TCPSocket *sock, string fname);
	void clientDel(TCPSocket *sock, string fname);
	void clientList(TCPSocket *sock, string fname);

	void sdfsHandler(TCPSocket *sock);

	/* Membership functions */
	int 	newMember(char * remote_ip);
	void 	updateContact(long long ts);
	void 	setMemberList(string s);
	void 	setMasterList(string s);
	string 	contactsToString();
	string	membersToString();
	string 	mastersToString();

	/* Utility funcitons */
	long long 	unixTimestamp();
	void 		log(string s, int flag = 0);
	void 		log(const char *fmt, ...);
	void 		plog(string s);
	void 		plog(const char *fmt, ...);
	void 		setSelfAddr();
	void 		setLogFile();
	bool 		dropMsg();
	bool 		prefixMatch(string org, string patt);

	/* Receiver handler functions */
	void joinHandler(char *remote_ip);
	void updateHandler(string msg);
	void heartbeatHandler(char *remote_ip);
	
};

class KV {
    public:
        double value;
        int id;

        KV(int id_, double val_) {
            value = val_;
            id = id_;
        }

        const bool operator < (const KV &r) const {
            return value < r.value;
        }

        const bool operator > (const KV &r) const {
            return value > r.value;
        }

};