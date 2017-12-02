#include "tcpsocket.hpp"
#include <sys/types.h>       // For data types
#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
#include <netdb.h>           // For gethostbyname()
#include <arpa/inet.h>       // For inet_addr()
#include <unistd.h>          // For close()
#include <netinet/in.h>      // For sockaddr_in
typedef void raw_type;       // Type used for raw data on this platform
#include <errno.h>             // For errno
#include <signal.h>

using namespace std;

// SocketException Code

SocketException::SocketException(const string &message, bool inclSysMsg)
  throw() : userMessage(message) {
  if (inclSysMsg) {
  userMessage.append(": ");
  userMessage.append(strerror(errno));
  }
}

SocketException::~SocketException() throw() {
}

const char *SocketException::what() const throw() {
  return userMessage.c_str();
}

// Function to fill in address structure given an address and port
static void fillAddr(const string &address, unsigned short port, 
           sockaddr_in &addr) {
  memset(&addr, 0, sizeof(addr));  // Zero out address structure
  addr.sin_family = AF_INET;       // Internet address

  hostent *host;  // Resolve name
  if ((host = gethostbyname(address.c_str())) == NULL) {
  // strerror() will not work for gethostbyname() and hstrerror() 
  // is supposedly obsolete
  throw SocketException("Failed to resolve name (gethostbyname())");
  }
  addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);

  addr.sin_port = htons(port);     // Assign port in network byte order
}

// Socket Code

Socket::Socket(int type, int protocol) throw(SocketException) {
  // Make a new socket
  if ((sockDesc = socket(PF_INET, type, protocol)) < 0) {
  throw SocketException("Socket creation failed (socket())", true);
  }
}

Socket::Socket(int sockDesc) {
  this->sockDesc = sockDesc;
}

Socket::~Socket() {
  ::close(sockDesc);
  sockDesc = -1;
}

string Socket::getLocalAddress() throw(SocketException) {
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
  throw SocketException("Fetch of local address failed (getsockname())", true);
  }
  return inet_ntoa(addr.sin_addr);
}

unsigned short Socket::getLocalPort() throw(SocketException) {
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
  throw SocketException("Fetch of local port failed (getsockname())", true);
  }
  return ntohs(addr.sin_port);
}

void Socket::setLocalPort(unsigned short localPort) throw(SocketException) {
  // Bind the socket to its port
  sockaddr_in localAddr;
  memset(&localAddr, 0, sizeof(localAddr));
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(localPort);
  int enable = 1;
  if (setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    printf("setsockopt(SO_REUSEADDR) failed\n");
  }

  if (bind(sockDesc, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0) {
    throw SocketException("Set of local port failed (bind())", true);
  }
}

void Socket::setLocalAddressAndPort(const string &localAddress,
  unsigned short localPort) throw(SocketException) {
  // Get the address of the requested host
  sockaddr_in localAddr;
  fillAddr(localAddress, localPort, localAddr);

  if (bind(sockDesc, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0) {
    throw SocketException("Set of local address and port failed (bind())", true);
  }
}

void Socket::cleanUp() throw(SocketException) {
}

unsigned short Socket::resolveService(const string &service,
                    const string &protocol) {
  struct servent *serv;        /* Structure containing service information */

  if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
  return atoi(service.c_str());  /* Service is port number */
  else 
  return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}

// CommunicatingSocket Code

CommunicatingSocket::CommunicatingSocket(int type, int protocol)  
  throw(SocketException) : Socket(type, protocol) {
}

CommunicatingSocket::CommunicatingSocket(int newConnSD) : Socket(newConnSD) {
}

void CommunicatingSocket::connect(const string &foreignAddress, unsigned short foreignPort) throw(SocketException) {
  // Get the address of the requested host
  sockaddr_in destAddr;
  fillAddr(foreignAddress, foreignPort, destAddr);

  // Try to connect to the given port
  if (::connect(sockDesc, (sockaddr *) &destAddr, sizeof(destAddr)) < 0) {
    throw SocketException("Connect failed (connect())", true);
  }
}

void CommunicatingSocket::send(const void *buffer, int bufferLen) throw(SocketException) {
  if (::send(sockDesc, (raw_type *) buffer, bufferLen, 0) < 0) {
    throw SocketException("Send failed (send())", true);
  }
}

int CommunicatingSocket::recv(void *buffer, int bufferLen) 
  throw(SocketException) {
  int rtn;
  if ((rtn = ::recv(sockDesc, (raw_type *) buffer, bufferLen, 0)) < 0) {
  throw SocketException("Received failed (recv())", true);
  }

  return rtn;
}

string CommunicatingSocket::getForeignAddress() 
  throw(SocketException) {
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getpeername(sockDesc, (sockaddr *) &addr,(socklen_t *) &addr_len) < 0) {
  throw SocketException("Fetch of foreign address failed (getpeername())", true);
  }
  return inet_ntoa(addr.sin_addr);
}

unsigned short CommunicatingSocket::getForeignPort() throw(SocketException) {
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getpeername(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
  throw SocketException("Fetch of foreign port failed (getpeername())", true);
  }
  return ntohs(addr.sin_port);
}

// TCPSocket Code

TCPSocket::TCPSocket() 
  throw(SocketException) : CommunicatingSocket(SOCK_STREAM, 
  IPPROTO_TCP) {
}

TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort)
  throw(SocketException) : CommunicatingSocket(SOCK_STREAM, IPPROTO_TCP) {
  connect(foreignAddress, foreignPort);
}

TCPSocket::TCPSocket(int newConnSD) : CommunicatingSocket(newConnSD) {
}

// TCPServerSocket Code

TCPServerSocket::TCPServerSocket(unsigned short localPort, int queueLen) 
  throw(SocketException) : Socket(SOCK_STREAM, IPPROTO_TCP) {
  setLocalPort(localPort);
  setListen(queueLen);
}

TCPServerSocket::TCPServerSocket(const string &localAddress, 
  unsigned short localPort, int queueLen) 
  throw(SocketException) : Socket(SOCK_STREAM, IPPROTO_TCP) {
  setLocalAddressAndPort(localAddress, localPort);
  setListen(queueLen);
}

TCPSocket *TCPServerSocket::accept() throw(SocketException) {
  int newConnSD;
  if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0) {
  throw SocketException("Accept failed (accept())", true);
  }

  return new TCPSocket(newConnSD);
}

void TCPServerSocket::setListen(int queueLen) throw(SocketException) {
  if (listen(sockDesc, queueLen) < 0) {
  throw SocketException("Set listening socket failed (listen())", true);
  }
}

int TCPSocket::sendStr(string in) {
  string lenStr = std::to_string(in.length());
  for (int i = 0; i < 10 - int(lenStr.length()); i++) in = " " + in;
  in = lenStr + in;
  try { 
    send(in.c_str(), in.length());
  } catch (...) {
    return 1;
  }
  return 0;
}

int TCPSocket::recvStr(string &result) {
  int RCVBUFSIZE = 5000;
  char recvBuffer[RCVBUFSIZE + 1];
  int totalBytes = 1000000;
  int bytesReceived = 0;
  int totalBytesReceived = 0;
  result = "";
  try {
    while (totalBytesReceived < totalBytes) {
    if ((bytesReceived = (recv(recvBuffer, RCVBUFSIZE))) <= 0) {
      //cerr << "Unable to read";
      //exit(1);
      return 1;
    }
    recvBuffer[bytesReceived] = '\0';
    if (totalBytesReceived == 0) {
      totalBytes = stoi(string(recvBuffer).substr(0, 10)) + 10;
      result += string(recvBuffer).substr(10, bytesReceived);
    } else {
      result += string(recvBuffer);
    }
    totalBytesReceived += bytesReceived;
    }
  } catch (...) {
    return 1;
  }
  return 0;
}

int TCPSocket::sendFile(string fname) {
  int TCPBUFSIZE = 4096, bytesRead = 0;
  FILE *fp = fopen(fname.c_str(), "rb");
  if (fp == NULL) {
    return 2;
  }
  fseek(fp, 0L, SEEK_END);
  int sz = ftell(fp);
  string lenStr = std::to_string(sz);
  int it = 10 - lenStr.length();
  for (int i = 0; i < it; i++) {
    lenStr = lenStr + " ";
  }
  try {
    send(lenStr.c_str(), lenStr.length());
    fseek(fp, 0, 0);
    int total = 0;
    while (true) {
      char buffer[TCPBUFSIZE + 100];
      bytesRead = fread(buffer, 1, TCPBUFSIZE, fp);
      buffer[bytesRead] = '\0';
      total += bytesRead;
      try {
        send(buffer, bytesRead);
      } catch (...) {
        return 1;
      }
      if (bytesRead < TCPBUFSIZE) break;
    }
  } catch (...) {
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
} 

int TCPSocket::recvFile(string fname) {
  int TCPBUFSIZE = 4096, totalBytes = 1000000, bytesReceived = 0, totalBytesReceived = 0;
  FILE *fp = fopen(fname.c_str(), "wb");
  if (fp == NULL) {
    return 2;
  }
  try {
    while (totalBytesReceived < totalBytes) {
      char recvBuffer[TCPBUFSIZE + 100];
      if ((bytesReceived = (recv(recvBuffer, TCPBUFSIZE))) <= 0) {
        //cerr << "Unable to read";
        //exit(1);
        return 1;
      }
      recvBuffer[bytesReceived] = '\0';
      if (totalBytesReceived == 0) {
        totalBytes = stoi(string(recvBuffer).substr(0, 10)) + 10;
        //plog("file size is %d", totalBytes);
        fwrite(recvBuffer+10 , sizeof(char), bytesReceived-10, fp);
      } else {
        fwrite(recvBuffer , sizeof(char), bytesReceived, fp);
      }
      totalBytesReceived += bytesReceived;
    }
  } catch (...) {
    fclose(fp);
    // Need to delete file
    return 1;
  }
  fclose(fp);
  return 0;
}



