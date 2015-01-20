// PixelOracleDatabase.cc

#include <fstream>
#include <iostream>

#include "PixelConfigDBInterface/include/PixelOracleDatabase.h"
#include "PixelConfigDBInterface/include/PixelDatabaseUtilities.h"

using namespace std;
using namespace oracle::occi;

Environment* PixelOracleDatabase::environment_ = 0;
StatelessConnectionPool* PixelOracleDatabase::connectionPool_  = 0;
int PixelOracleDatabase::numberOfConnects_ = 0;

PixelOracleDatabase::PixelOracleDatabase(){
//   cout << __LINE__ << "]\t[PixelOracleDatabase::PixelOracleDatabase()]" << endl;
  connections_ = 0;
  //A thread that attempts to lock a non-recursive mutex it already owns (has locked)
  //will receive a deadlock indication and the attempt to lock the mutex will fail.
  //Using a recursive mutex avoids this problem, but the thread must ensure that it
  //unlocks the mutex the appropriate number of times. Otherwise no other threads will be
  //able to lock the mutex.
  //A recursive mutex must be used if there are more functions locking the mutex or if 
  //the function that locks is recursive. 
  //Here it is necessary because connect and disconnect are the only functions that locks the mutex, 
  //but they are not innested 
  pthread_mutex_t lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
  environment_ = 0;
  connectionPool_ = 0;
  mutexLock_ = lock;
  numberOfConnects_ = 0;
}

PixelOracleDatabase::~PixelOracleDatabase(){
//   cout << "PixelOracleDatabase::~PixelOracleDatabase" << endl;
  disconnect();

}

bool PixelOracleDatabase::connect(){
  std::string mthn = "]\t[PixelOracleDatabase::connect()]\t\t\t    " ;
  cout << __LINE__ << mthn << endl;
  pthread_mutex_lock(&mutexLock_);
  if (numberOfConnects_ == 0) {
// 	load_variable("sql_connection_fnal",m_connection);
// 	load_variable("sql_username_fnal",m_username);
// 	load_variable("sql_password_fnal",m_password);
// 	load_variable("sql_connection_cern",m_connection);
//	load_variable("sql_username_cern",m_username);
// 	load_variable("sql_password_cern",m_password);
   string connection;
   string username;
   string password;
   load_variable("sql_connection_dev",connection);
   load_variable("sql_username_dev",username);
   load_variable("sql_password_dev",password);
   cout << __LINE__ << mthn << "Connnection Information:"      << endl;
   cout << __LINE__ << mthn << "  connection = " << connection << endl;
   cout << __LINE__ << mthn << "  username   = " << username   << endl;
   cout << __LINE__ << mthn << "Connecting..."  	       << endl;
   environment_ = Environment::createEnvironment(Environment::THREADED_UNMUTEXED);
   cout << __LINE__ << mthn << "Environment Created"	       << endl; 
   bool timeout = false;
   while(!timeout)
     {
       try
         {
  	   connectionPool_ = environment_->createStatelessConnectionPool(username, 
									 password, 
									 connection,
									 10,
									 1,
									 1,
									 StatelessConnectionPool::HOMOGENEOUS);
	   timeout = true;
// 	   std::cout << __LINE__ << mthn << "Opened connections: " 
// 	             <<  connectionPool_->getOpenConnections() << endl;
   	 } 
       catch(SQLException sqle)
         {
	   std::cout << __LINE__ << mthn << sqle.getMessage() << std::endl ;
	 }
   } 

//    cout << __LINE__ << mthn << "Connection pool created, setting busy options..." << endl;

   connectionPool_->setBusyOption(StatelessConnectionPool::NOWAIT); 
  }

  numberOfConnects_++;
  pthread_mutex_unlock(&mutexLock_);
//   cout << __LINE__ << mthn << "Done" << endl ;

//   if (m_connections.connect()){
//     return (connections_ = this->getConnection());
//   connections_ = this->getConnection();
  return true;
//   }
//   else{
//     return false;
//   }
}

Connection* PixelOracleDatabase::getConnection(){
//  connections_ = connectionPool_->getConnection();
//   cout << __LINE__ << "]\t[PixelOracleDatabase::getConnection()]\t\t\tBusy conn:" << connectionPool_->getBusyConnections() << endl; 
  return connectionPool_->getConnection();
 }

void PixelOracleDatabase::releaseConnection(Connection* conn){
  connectionPool_->releaseConnection(conn);
}

void PixelOracleDatabase::disconnect(){
  
//   std::cout << __LINE__ << "]\t[PixelOracleDatabase::disconnect()]" << std::endl << std::endl << std::endl;
//  connections_ = 0;
  if (connections_) {
    this->releaseConnection(connections_);
    connections_ = 0;
  }
  pthread_mutex_lock(&mutexLock_);
  if (--numberOfConnects_ == 0){
    if (environment_){
      Environment::terminateEnvironment(environment_);
      environment_ = 0;
    }
  }
  pthread_mutex_unlock(&mutexLock_);
}

