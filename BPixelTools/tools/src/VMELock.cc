#include <iostream>
#include "VMELock.h"

//-----------------------------------------------------------

VMELock::VMELock(const int use){
  useLock=use;
  if (use==0) return;
  semid = semget (KEY, 0, IPC_PRIVATE);
  if (semid < 0) {
    umask(0);
    semid = semget (KEY, 1, IPC_CREAT | IPC_EXCL | PERM);
    if (semid < 0) {
      std::cout <<"Unable to create semaphore ..." << std::endl;
      //return -1;
    }
    std::cout << " semaphore id: " << semid << std::endl;
    semctl (semid, 0, SETVAL, (int) 1);
  }else{
    int pid=semctl (semid, 0, GETPID, 0);
    std::cout << "semaphore already exists, created by PID " << pid << std::endl;
  }
  semaphore.sem_num=0;
}

 
//-----------------------------------------------------------

bool VMELock::acquire(){
  if (useLock==0) return true;
  if (DEBUG) std::cout << ">>>>>>>>>>>>>>> acquiring lock " << semid << " " << semaphore.sem_num << std::endl;
  semaphore.sem_op = -1;
  semaphore.sem_flg = SEM_UNDO;
  if( semop (semid, &semaphore, 1) == -1) {
    perror(" VMELock::acquire ");
    std::cout << "!!!!!!! Warning  !!!!!   failed to acquire lock" << std::endl;
    return false;
  }else{
    return true;
  }
}

//-----------------------------------------------------------

void VMELock::release(){
  if (useLock==0) return;
  if (DEBUG) std::cout << "<<<<<<<<<<<<<<< releasing lock " << semid << " " << semaphore.sem_num  << std::endl;
  semaphore.sem_op = 1;
  semaphore.sem_flg = SEM_UNDO;
  if( semop (semid, &semaphore, 1) == -1) {
    perror(" VMELock::release ");
    std::cout << "!!!!!!! Warning  !!!!!   failed to release lock" << std::endl;
  }
}
