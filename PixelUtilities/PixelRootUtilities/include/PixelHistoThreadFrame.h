#ifndef _PixelHistoThreadFrame_h_
#define _PixelHistoThreadFrame_h_ 

#include <vector>

class TThread;

class PixelHistoThreadFrame{
 public:
  
  PixelHistoThreadFrame(int nOfThreads=1);
  virtual ~PixelHistoThreadFrame();
 
  static void thread0(void* arg); // functions running as threads
//  static void thread1(void* arg);  

  virtual void userFunc0();	// user functions called from Thread0
//  virtual void userFunc1(); 	

  virtual int startThreads(); // launch all threads 
  virtual int stopThreads(); // stop all threads
 
  bool getThreadsRun(){return threadsRun_;}

 protected:
  bool threadsRun_; // flags for quick abort of loops within threads
                    // may also use different flags for each thread	
	std::vector<bool> funcRunning_;

 private:
  std::vector<TThread*> pThreads_;		// thread pointer
	int nOfThreads_;
};

#endif
