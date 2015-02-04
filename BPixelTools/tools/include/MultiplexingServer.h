#ifndef MULTIPLEXINGSERVER
#define MULTIPLEXINGSERVER

#include <stdio.h>
#include <string>
#include <iostream>
#include <sstream>
#include <strings.h>
#include <string.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "SimpleConnection.h"
#include "BaseCommand.h"



#define NIPQUAD(addr) \
         ((unsigned char *)&addr)[0], \
         ((unsigned char *)&addr)[1], \
         ((unsigned char *)&addr)[2], \
         ((unsigned char *)&addr)[3]






extern void Gl_histinit(char*);
//extern char *Getline(const char*);
extern char *Getlinem(int, const char*);
extern void  Gl_histadd(const char *);



class MultiplexingServer{
 private:
  // state variables for the eventloop
  bool lfirst;
  bool reentrant;
  std::string request;
  SimpleConnection * connection;
  std::string prompt;


  //configuration
  int portnumber;
  BaseCommand * cmd;
  bool ownCommand;
  bool console;

  // connections
  int sockfd;
  static const int nMaxClient=5;
  SimpleConnection client[nMaxClient];




 public:
  MultiplexingServer();
  MultiplexingServer(BaseCommand * aCmd, const bool bg=false);
  ~MultiplexingServer (); 
  void open(int port);
  BaseCommand* getCmd(){return cmd;};
  bool eventloop();
  void setPrompt(const std::string s);
  void shutdownServer();

 private:
  void init(const bool bg);
  SimpleConnection* get(std::string &, std::string);
  bool acceptClient();
};
#endif
