#ifndef VMELOCK
#define VMELOCK


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#define PERM 0666    
#define KEY  123458L
//#define KEY  0x30080018  // VME
#define DEBUG 0


class VMELock{

 private:
  struct sembuf semaphore;
  int semid;
  int useLock;

public:
  VMELock(const int use=1);
  bool acquire();
  void release();
};
#endif
