// PixelOracleDatabase.h
#ifndef PixelOracleDatabase_h
#define PixelOracleDatabase_h

#include <occi.h>
#include <sstream>
#include <pthread.h>

class PixelOracleDatabase {
 public:
  PixelOracleDatabase();
  ~PixelOracleDatabase();
  bool connect();
  void disconnect();
  oracle::occi::Connection *getConnection();
  void releaseConnection(oracle::occi::Connection *conn);
  int getNumberOfConnections(){return numberOfConnects_;}
 private:
  static oracle::occi::Environment *environment_;
  static oracle::occi::StatelessConnectionPool *connectionPool_;
//  static oracle::occi::ConnectionPool *connectionPool_;
  static int numberOfConnects_;
  pthread_mutex_t mutexLock_;
  oracle::occi::Connection *connections_;
};

#endif
