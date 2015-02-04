#include "MultiplexingServer.h"
#include "SimpleCommand.h"

#include <sys/ioctl.h>  // keyboard stuff


MultiplexingServer::MultiplexingServer(){
  init(false);
  cmd=new SimpleCommand();
  ownCommand=true;
}

void MultiplexingServer::init(const bool bg){
  if(bg){
    console=false;
  }else{
    console=true;
    Gl_histinit(".hist");
  }
  portnumber=0;
  prompt=">";
  connection=NULL;
  reentrant=false;
}


MultiplexingServer::MultiplexingServer(BaseCommand * aCmd,const  bool bg){
  init(bg);
  ownCommand=false;
  cmd=aCmd;
}


 
MultiplexingServer::~MultiplexingServer(){
  if(ownCommand) delete cmd;
}



void MultiplexingServer::setPrompt(const std::string s){
  prompt=s;
  for(int i=0; i<nMaxClient; i++){
    if( client[i].isOpen()){
      client[i].setPrompt(prompt);
    }
  }
}



void MultiplexingServer::open(int port){
  portnumber=port;

  //bind 
  struct sockaddr_in serv_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    perror("ERROR opening socket");
    exit(1);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  while (bind(sockfd, (struct sockaddr *) &serv_addr,
	      sizeof(serv_addr)) < 0){
    perror("Error on binding"); // keep trying
    sleep(1);
  }
  int errcode=listen(sockfd,5);
  if (errcode == -1) {
    perror ("Error listening to bound socket");
    exit(0);
  }

  if (fcntl(sockfd, F_SETFL, O_NDELAY) < 0) {
    perror("Can't set socket to non-blocking");
    exit(0);
  }

  printf("waiting for clients\n");

}


bool MultiplexingServer::acceptClient(){

  struct sockaddr_in cli_addr;
  socklen_t clilen = sizeof(cli_addr);


  int newsockfd = accept(sockfd, 
			 (struct sockaddr *) &cli_addr, 
			 &clilen);
  if (newsockfd !=-1){	    
    printf ("client connecting from %d.%d.%d.%d\n",
	    NIPQUAD(cli_addr.sin_addr.s_addr));

    // find an available slot
    for(int i=0; i<nMaxClient; i++){
      if (!(client[i].isOpen())){
	client[i].open(newsockfd);
	client[i].send("Howdy");
	client[i].setNonblocking();
	printf("accepted client connection in slot %d\n",i);
	return true;
      }
    }
    printf("no free slot available \n");
    return false;
  } else {
    return false;
  }
}





SimpleConnection * MultiplexingServer::get(std::string& request, std::string prompt ){
  while(true){
    
    if(console){// handle keyboard input
      int n;
      char *p;
      ioctl(STDIN_FILENO,FIONREAD, &n);  // has a key been pressed?
      if((n>0)&& (p = Getlinem(1,NULL))){
	Gl_histadd(p);
	if((strlen(p)>0)&&p[strlen(p)-1]=='\n'){ p[strlen(p)-1]=0;}
	request=p;
	return NULL;
      }
    }
      
    // look for new clients trying to connect
    acceptClient();

    // handle request from sockets
    for(int i=0; i<nMaxClient; i++){
      if( (client[i].isOpen()) && (client[i].poll()) ){
	client[i].get(request);
	return &(client[i]);
      }
    }

    //usleep(10000);
    usleep(100);
  }
}


void MultiplexingServer::shutdownServer(){
  printf("shutting down server\n");
  for(int i=0; i<nMaxClient; i++){
    if( client[i].isOpen()){
      client[i].finish("server is shutting down");
    }
  }
  reentrant=false;
  if(console){Getlinem(2, NULL);}
}



bool MultiplexingServer::eventloop(){
  // eventloop, generator style

  do{

    // fall through this part when you return from event handling
    if(!reentrant){
      if(connection){
	connection->flush();
	connection->done();
	connection->restore(cout);
      }else if(console){
	Getlinem(-1,prompt.c_str());
      }
      
      connection=get(request,prompt.c_str());
      
      if(connection){
	connection->redirect(cout);
      }
    }

    // 
    //printf("parsing request %s, reentrant=%d\n",request.c_str(),reentrant);
    if(reentrant||cmd->Parse(request)){// Parse only executes when not reentrant
      do{
	if (!reentrant && cmd->Keyword("shutdown")){
	  shutdownServer();
	    return false;  // it's over
	}
	
	// this is where we would normally call the event handler
	// instead we return to the caller and make sure we
	// continue right here upon the next call
	if(! reentrant ){
	  reentrant=true;
	  return true;    // handle this request and call me back
	}else{
	  reentrant=false; // continue after having been called back
	}
	
	if(connection){
	  connection->flush();
	}
      }while(cmd->Next());
    }else{
      if(connection){
	connection->flush();
      }
    }


    if(connection && ( cmd->Exit() || connection->died()) ){
      connection->restore(cout);
      connection->finish();
      connection=NULL;
    }else if(cmd->Exit()){
      //exit issued from the command line is like a shutdown
      shutdownServer();
      break;
    }
  }while(true);
  return false;  // it's over, nothing left to do
  
}


