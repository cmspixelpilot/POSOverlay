#ifndef _PIXELTIMER_H_
#define _PIXELTIMER_H_
#include <math.h>
#include <sys/time.h>
#include <iostream>

class PixelTimer{
    
 public:
    
    PixelTimer();
    virtual ~PixelTimer();

    void start(const std::string msg="");
    void stop(const std::string msg="");
    unsigned int ntimes() const;
    double  tottime() const;
    double  avgtime() const;
    double  rms() const;

    bool started() {return startflag_;}
    void setVerbose(bool verbosity) {verbose_=verbosity;}
    void setName(std::string name) {name_=name;}
    void printTime(const std::string msg="");

    void reset();

 private:

    unsigned int ntimes_;
    double ttot_;
    double ttotsq_;

    timeval tstart_;

    bool startflag_;
    bool verbose_;
    std::string name_;
    void printTime(const timeval t, const std::string msg="") const;

};


#endif
