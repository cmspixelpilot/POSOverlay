
#include <PixelUtilities/PixelTestStandUtilities/include/PixelTimer.h>
#include <math.h>
#include <sys/time.h>
#include <ctime>

PixelTimer::PixelTimer(){

  ntimes_=0; 
  ttot_=0.0;
  ttotsq_=0.0;

  startflag_=false;
  verbose_=false;
  name_="";

}
 
PixelTimer::~PixelTimer(){}

void PixelTimer::start(const std::string msg) {

  gettimeofday(&tstart_,0);

  startflag_=true;
  if (verbose_) printTime(tstart_,msg);
}

void PixelTimer::reset() {
  ntimes_=0; 
  ttot_=0.0;
  ttotsq_=0.0;
  startflag_=false;
}

void PixelTimer::stop(const std::string msg) {
  startflag_=false;
  timeval tstop;
  gettimeofday(&tstop,0);
  if (verbose_) printTime(tstop,msg);
  float tsec=tstop.tv_sec-tstart_.tv_sec;
  float tusec=tstop.tv_usec-tstart_.tv_usec;
  if (tusec<0){
    tusec+=1000000.0;
    tsec-=1.0;
  }
  double tmp=tsec+tusec/1000000.0;
  ttot_+=tmp;
  ttotsq_+=tmp*tmp;
  ntimes_++;
}

unsigned int PixelTimer::ntimes() const {
  return ntimes_;
}

double  PixelTimer::tottime() const {

  return ttot_;

}

double  PixelTimer::avgtime() const {
  if (0==ntimes_) return 0;
  return ttot_/ntimes_;

}

double  PixelTimer::rms() const {
  if (0==ntimes_) return 0;
  return sqrt((ttot_*ttot_-ttotsq_))/ntimes_;

}    

void PixelTimer::printTime(const std::string msg) {

  timeval tnow;
  gettimeofday(&tnow,0);
  printTime(tnow,msg);
}

void PixelTimer::printTime(const timeval t, const std::string msg) const {

  char buffer[10];
  time_t mytime=t.tv_sec;
  struct tm *timeinfo = gmtime(&mytime); 
  strftime (buffer,10,"%H:%M:%S",timeinfo);
  std::cout<<"["<<name_<<": "<<msg<<"] "<<buffer<<" "<<t.tv_usec<<std::endl;

}
