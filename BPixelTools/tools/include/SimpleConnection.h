#ifndef SIMPLECONNECTION
#define SIMPLECONNECTION
// /* A simple server in the internet domain using TCP
//  shamelessly copied from http://www.linuxhowtos.org/C_C++/socket.htm
//  and modified */

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>


class SimpleConnection
{

 private:
  
  static const int maxPacketLength= 4096;
  static const int headerSize=5; 
  static const char* packetFormat;

  // packet data
  int nrecvd;
  char buffer[maxPacketLength+1];
  char header[headerSize+1];
  int len;
  //std::string message;
  
  //
  std::streambuf* sbuf;
  bool nonblocking;
  int fd;
  std::string prompt;

  void sendPacket(const char& type, const std::string s);
  void error(const char*  s);
  int safeRead(int fd, void *buf, size_t count){
    try{ return read(fd,buf,count); }catch(...){return -99;}
  }

 public:

  std::ostringstream out;
 
  SimpleConnection(){fd=0; nrecvd=0;}
  SimpleConnection(int socketFD){fd=socketFD; nrecvd=0;}
  ~SimpleConnection();
  void open(int socketFD){fd=socketFD; nrecvd=0;};
  void send(std::string s);
  void talk(std::string s);
  void flush();
  void finish(const std::string s="bye bye");
  void done();
  void get(std::string& s);
  void setPrompt(std::string s);
  void redirect (std::ostream& strm);
  void restore (std::ostream& strm);
  bool died(){return (fd==0);};
  bool isOpen(){return (fd!=0);};
  int setNonblocking();
  bool poll();
};


#endif
