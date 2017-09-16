#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <stdexcept>
#include <algorithm>
using namespace std;
#define PORT "3490"  // the port users will be connecting to
#define BACKLOG 20	 // how many pending connections queue will hold
#define MAXDATASIZE 8192 // max number of bytes we can get at once 


void sigchld_handler(int s){
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET){
		return &(((struct sockaddr_in*)sa)->sin_addr);
	} else {
		return &(((struct sockaddr_in6*)sa)->sin6_addr);
	}
}

string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int server_start(){
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	char s[INET6_ADDRSTRLEN];	
	
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sigaction sa;
	int yes=1, numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		// I change it from REUSEADDR to REUSEPORT
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while(1) {  
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}
		inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);
		if (!fork()) { 
			close(sockfd); 

			string query = "";
			char buf[MAXDATASIZE];
			if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			buf[numbytes] = '\0';
			printf("Receive: %s\n", buf);
			string query_cmd = buf;
			string res = exec(query_cmd.c_str());
			/*
			size_t bytes_sent = 0;
			while (numbytes = send(new_fd, res.c_str() + bytes_sent, res.length(), 0))
			*/
			//if (send(new_fd, res.c_str(), res.length(), 0) == -1) perror("send");
			ssize_t total = 0;
			int scount = 0;
			while (true){
				ssize_t sent_bytes = send(new_fd, res.c_str() + total, MAXDATASIZE - 1, 0);
				if (sent_bytes == -1) perror("send");
				//printf("sent: %ld\n", sent_bytes);
				//printf("count: %d\n", ++scount);
				
				if (sent_bytes < MAXDATASIZE - 1) {
					break;
				}
				
				total += sent_bytes;
				//printf("total: %ld\n", total);
				if (total >=  res.length()) {
					break;
				}
			}
			printf("total is: %ld\n", total);
			printf("length: %lu\n", res.length());
			printf("line count: %ld\n", count(res.begin(), res.end(), '\n'));

			close(new_fd);
			exit(0);
		} else {
			close(new_fd);  
		}
	}
	return 0;
}

int main(){
	server_start();
	return 0;
}
