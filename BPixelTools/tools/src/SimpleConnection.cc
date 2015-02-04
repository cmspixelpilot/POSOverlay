// /* A simple connection in the internet domain using TCP
//  shamelessly copied from http://www.linuxhowtos.org/C_C++/socket.htm
//  and modified */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/termios.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <sstream>
//#include <stropts.h>

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <poll.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <time.h>
#include <signal.h>

#include "SimpleConnection.h"

//const int SimpleConnection::headerSize=5;  
const char* SimpleConnection::packetFormat="%c%4d%s";
//const int SimpleConnection::maxPacketLength = 4096;



void SimpleConnection::sendPacket(const char& type, const std::string s){
  if(fd==0){
    std::cout << "sendPacket called with invalid fd" << std::endl;
    return;
  }
  // send a message but don't send "done"
  char packet[maxPacketLength+1];
  unsigned int m=0;
  if(s.length()==0){
    if (type==' '){
      return; // swallow empty data packets, no need to send those
    }
    snprintf(packet,maxPacketLength,packetFormat,type,0,"");
#ifdef DEBUG
    std::cerr << "SimpleConnection::sending packet " << packet << std::endl;
#endif

    int n=0;
    try {
      n = write(fd,packet,headerSize);
      if (n < 0){
	error("ERROR writing header to socket");    
	return;
      }
    } 
    catch(...){
      // why can't I catch (SIGPIPE){
      printf("execption caught in SimpleConnection::sendPacket\n");
    }
    return;
  }else{
    while(m<s.length()){
      int l=s.length()-m;
      if (l+headerSize>maxPacketLength){
	l=maxPacketLength-headerSize;
      }
      snprintf(packet,maxPacketLength,packetFormat,type,l,s.substr(m,l).c_str());
#ifdef DEBUG
      std::cerr << "SimpleConnection::sending packet " << packet << std::endl;
#endif
      int n;
      try{
	n = write(fd,packet,l+headerSize);
	if (n < 0){
	  error("ERROR writing message to socket");
	  return;
	}
	m+=l;
      }catch (...){
	printf("execption caught in SimpleConnection::sendPacket\n");
	return;
      }
    }
  }
}


void SimpleConnection::send(std::string s){
  talk(s);
  done();
}

void SimpleConnection::talk(std::string s){
  // send a message but don't send "done"
  sendPacket(' ',s);
}

void SimpleConnection::done(){ 
  sendPacket('D',"");
}


void SimpleConnection::flush(){
  // flush the stream buffer through the socket (but don't send "done")
  //out << std::ends;
  talk(out.str()); 
  out.str("");      // clear the buffer
}


void SimpleConnection::get(std::string& s){
  // get the client request
#ifdef DEBUG
  printf("SimpleConnection::waiting for data\n");
#endif
  if (!nonblocking){
    int n = safeRead(fd,header,headerSize);

    if (n <= 0) {
      error("ERROR reading from socket");
      return;
    }
    header[headerSize]=0;
    len=atoi(header);
    if(len>0){
      n = safeRead(fd,buffer,len);
      if (n<=0) {
	error("Error reading from socket");
	return;
      }
    }
    buffer[len]=0;
  }

  //printf("get: header=%s  len=%d,  message=%s\n",header,len,buffer);
  // copy the message buffer (may be zero length)
  s=buffer;
  if(header[0]==' '){
    return;
  }else if(header[0]=='C'){
    finish();
    return;
  }else if(header[0]=='P'){
    return;
  }else{
    s="";
    printf("malformed packet received: length=%d [%s]\n",len,header);
  }
#ifdef DEBUG
  printf("received data: %s\n",s.c_str());
#endif
}


void SimpleConnection::finish(std::string message){
  if (fd==0) return;
  printf("closing connection\n");
  try{
    sendPacket('C',message);
    sleep(1);
  }catch(...){
    printf("exception caught during send in SimpleConnection::finish\n");
  }
  try{
    close(fd);
  }catch(...){
    printf("exception caught during close in SimpleConnection::finish\n");
  }
  fd=0;
  printf("successfully closed\n");
}


SimpleConnection::~SimpleConnection(){
  if(fd!=0){
    finish();
  }
}


void SimpleConnection::setPrompt(std::string s){ 
  if(prompt==s) return;  // only send when it changed
  prompt=s;
  sendPacket('P',s);
}


void SimpleConnection::redirect (std::ostream& strm)
{
  sbuf = strm.rdbuf();
  strm.rdbuf(out.rdbuf());
}

void SimpleConnection::restore (std::ostream& strm)
{
    strm.rdbuf(sbuf);
}

void SimpleConnection::error(const char*  s){
  //  if(errno!=EAGAIN){// be quiet about "temporarily unavailable"
  perror(s);
  std::cout<< "connection terminated abnormally" << std::endl;
  fd=0;  // don't try closing properly, dead already
}



bool SimpleConnection::poll(){
  //returns true when a complete packet is available

  // get the header first
  if(nrecvd<headerSize){
    int n;
    n = safeRead(fd,&(header[nrecvd]),headerSize-nrecvd);
    if ((n<0)&&(errno==EAGAIN)){
      return false;
    }else if(n>0){
      nrecvd+=n;
      if(nrecvd<headerSize){
	return false;
      }else{
	// header is complete
	len=atoi(header);
	// don't return yet, there might be more data available
      }
    }else{
      error("Error reading header from socket");// will also close the connection
      return false;
    }
  }
  // done with the header, get the message

  int n;
  if(headerSize+len-nrecvd>0){
    n = safeRead(fd,&(buffer[nrecvd-headerSize]),headerSize+len-nrecvd);
    if((n<0)&&(errno==EAGAIN)){
      return false;
    }else if(n<0){
      error("Error reading message from socket");
      return false;
    }
    nrecvd+=n;
  }

  if(nrecvd==(headerSize+len)){
      buffer[len]=0;
      nrecvd=0;  // reset for the next packet
      //printf("poll: received %s\n",buffer);
      return true;
  }else{
    return false; // still incomplete
  }
  
}

int SimpleConnection::setNonblocking()
{
    int flags;
    nonblocking=true;
    nrecvd=0;
    len=0;

    /* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}     
