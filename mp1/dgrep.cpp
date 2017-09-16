#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <thread>
using namespace std;

#define PORT "3490" // the port client will be connecting to 
#define MAXDATASIZE 8192 // max number of bytes we can get at once 
#define VMNUM 11

string vm[VMNUM];
int line_count[VMNUM]; 
int total_line;
thread threads[VMNUM];

void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	} else {
		return &(((struct sockaddr_in6*)sa)->sin6_addr);		
	}
}

string add_escape(char *str) {
	string ret = "";
	for (int i = 0; i < strlen(str); i++){
		if (str[i] == '"') {
			ret += "\\";
		}
		ret += str[i];
	}
	return ret;
}

string grep_query(int id, int argc, char*argv[]){
	string ret = "grep";
	for (int i = 1; i < argc-1; i++) {
		if (i == argc - 2){
			ret = ret + " \"" + add_escape(argv[i]) + "\"";
		} else {
			ret = ret + " " + add_escape(argv[i]);
		}
	}
	string file = argv[argc-1];
	int pos = file.find(".log");
	file = file.substr(0, pos) + to_string(id) + file.substr(pos, file.length() - pos);
	return ret + " " + file;
}


int query(int id, string cmd) {
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	const char *server = vm[id].c_str();

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			//perror("client: connect");
			close(sockfd);
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
	//printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); 

	//printf("id: %d's cmd is %s\n", id, cmd.c_str());
	// Echo service;
	if (send(sockfd, cmd.c_str(), cmd.length(), 0) == -1) perror("send");

	string result = "";
	int tmp = 0, scount = 0;
	while ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) > 0) {
		if (numbytes == -1) {
			perror("recv");
			exit(1);				
		}
		scount++;
		//printf("%d %lu %d %d\n", tmp, result.length(), numbytes, scount);
		tmp += numbytes;
		if (numbytes < MAXDATASIZE-1) {
			buf[numbytes] = '\0';
		} 
		result += buf;
	}

	//printf("machine %d starting writing\n", id);
	FILE *f;  
	char filename[20];
	sprintf(filename, "grep.%02d.txt", id); 
	f = fopen(filename, "w");  
	fprintf(f, "%s", result.c_str());  
	fclose(f);
	
	line_count[id] =  count(result.begin(), result.end(), '\n');
	

	close(sockfd);
	return 0;
}


int init(int argc, char *argv[]){
	vm[0] = "localhost";
	for (int i = 1; i <= 10; i++){
		char tmp[100];
		sprintf(tmp, "grep.%02d.log", i);
		remove(tmp);
		sprintf(tmp, "fa17-cs425-g59-%02d.cs.illinois.edu", i);
		vm[i] = tmp;

	}
	return 0;
}

int main(int argc, char *argv[]){
	init(argc, argv);

	for (int i = 1; i <= 10; i++) {
		threads[i] = thread(query, i, grep_query(i, argc, argv));
	}

	for (int i = 1; i <= 10; i++) {
		threads[i].join();
	}
	
	for (int i = 1; i <= 10; i++) {
		total_line += line_count[i];
		printf("Grep %8d lines from machine %d\n", line_count[i], i);
	}
	printf("%d\n", total_line);
}
