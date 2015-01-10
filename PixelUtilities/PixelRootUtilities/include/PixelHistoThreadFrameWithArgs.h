#ifndef _PixelHistoThreadFrameWithArgs_h_
#define _PixelHistoThreadFrameWithArgs_h_ 

#include <vector>
#include <string>

class TThread;
class ThreadArgs;

class PixelHistoThreadFrameWithArgs{
 public:
  
  PixelHistoThreadFrameWithArgs(int nOfThreads=1);
  virtual ~PixelHistoThreadFrameWithArgs();
 
  static void thread0(void* arg); // functions running as threads
//  static void thread1(void* arg);  

  virtual void userFunc0(int &threadNumber);	// user functions called from Thread0

  virtual int startThreads(); // launch all threads 
  virtual int stopThreads(); // stop all threads
 
  bool getThreadsRun(){return threadsRun_;}

 protected:
  bool threadsRun_; // flags for quick abort of loops within threads
                    // may also use different flags for each thread	
	std::vector<bool> funcRunning_;

 private:
  std::vector<TThread*> pThreads_;		// Thread pointer
  std::vector<ThreadArgs*> pThreadArgs_;		// Thread arguments

	int nOfThreads_;
};

//class ThreadArgs{
// public:
//	ThreadArgs(PixelHistoThreadFrameWithArgs *frame, std::string values) : frame_(frame), values_(values){;}
//	~ThreadArgs();
//	PixelHistoThreadFrameWithArgs *getFrame(){return frame_;}
//	std::string                    getValues(){return values_;}
//	std::vector<std::pair<std::string,std::string> > getArgsVector();
// private:	
//	PixelHistoThreadFrameWithArgs *frame_;
//	std::string                    values_;
//};

class ThreadArgs{
 public:
	ThreadArgs(PixelHistoThreadFrameWithArgs *frame, int index) : frame_(frame), index_(index){;}
	~ThreadArgs();
	PixelHistoThreadFrameWithArgs *getFrame(){return frame_;}
	int                            getIndex(){return index_;}
 private:	
	PixelHistoThreadFrameWithArgs *frame_;
	int                            index_;
};

#endif
