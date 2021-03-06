#include "Pdf.h"
#include "Observable.h"
#include "Tools.h"

namespace TOOLS {
  void makeTimeStamp( std::string & stamp ) {
    time_t curtime;
    time(&curtime);
    struct tm *loctime;
    loctime = localtime(&curtime);
    makeTimeStamp( stamp, loctime );
  }
  void makeTimeStamp( std::string & stamp, time_t t ) {
    struct tm *timeStruct = localtime(&t);
    makeTimeStamp( stamp, timeStruct );
  }
  void makeTimeStamp( std::string & stamp, struct tm *t ) {
    static char ts[32];
    strftime( ts, 32,"%d/%m/%Y %H:%M:%S",t);
    stamp = ts;
  }

  void calcIntRange(const OBS::Base & obs, double scale, double & xmin, double & xmax ) {
    const PDF::DISTYPE dist  = obs.getPdfDist();
    const double       mean  = obs.getObservedValue();
    const double       sigma = obs.getPdfUseSigma();
    bool positive = true; // only positive numbers in range
    //    int    nprev;
    int    n     = 0;
    int    nxmin = -1;
    int    nxmax = -1;
    double ptot  = 0.0;
    double p;
    const double maxp = 0.9999;
    const double minp = 1.0-maxp;
    //
    if (dist==PDF::DIST_CONST) {
      xmin  = mean;
      xmax = mean;
    } else {
      if (dist==PDF::DIST_LOGN) positive = false; // Log normal must allow negative values
      switch (dist) {
      case PDF::DIST_GAUS2D:
      case PDF::DIST_GAUS:
      case PDF::DIST_LOGN:
        xmin = mean - scale*sigma;
        xmax = mean + scale*sigma;
        break;
      case PDF::DIST_FLAT:
        // always full range
        TOOLS::calcFlatRange( mean, sigma, xmin, xmax );
        break;
      case PDF::DIST_POIS:
        // find min and max range of poisson
        // this is defined by maxp above
        // xmin  : max N for wich sum( p(n) ) < 1.0-maxp
        // xmax : min N for wich sum( p(n) ) > maxp
        while (nxmax<0) {
	  //          nprev=n;
          p = PDF::gPoisson.getVal( n, mean )*obs.aprioriProb(static_cast<double>(n));
          ptot += p;
          if ((n==0) && (ptot>minp)) nxmin=0;
          if (nxmin<0) {
            if (ptot<minp) nxmin=n;
          }
          if (ptot>maxp) nxmax = n;
          n++;
          // if (nprev>n) { // just a STUPID test; can be done better...
          //   std::cerr << "Infinite loop caugh in Tools::calcIntRange() for Poisson - brutal exit" << std::endl;
          //   exit(-1);
          // }
        }
        //
        xmin  = static_cast<double>(nxmin);
        xmax = static_cast<double>(nxmax);
        break;
      default: // ERROR STATE
        xmin  = 0;
        xmax = 0;
        std::cerr << "Tools::calcIntRange() -> Unknown pdf type = " << dist << std::endl;
        break;
      }
    }
    if (positive && (xmin<0)) {
      xmax = xmax-xmin;
      xmin = 0;
    }
  }
  //
  // class Timer members
  //
  void Timer::startTimer(const char *msg) {
    //
    m_stopTime = 0;
    m_estTime  = 0;
    //
    time(&m_startTime);
    m_runningTime = true;
    //
    if (msg) {
      std::string tstamp;
      TOOLS::makeTimeStamp( tstamp, m_startTime );
      std::cout << msg << tstamp << std::endl;
    }
  }

  bool Timer::checkTimer(int dt) {
    time_t chk;
    int delta=0;
    if (m_runningTime) {
      time(&chk);
      delta = int(chk - m_startTime);
    }
    return (delta>dt); // true if full time has passed
  }

  void Timer::stopTimer() {
    if (m_runningTime) {
      time(&m_stopTime);
      m_runningTime = false;
    }
  }

  void Timer::printTime(time_t t, const char *msg ) {
    std::string tstamp;
    makeTimeStamp( tstamp, t );
    if (msg) std::cout << msg << std::flush;
    std::cout << tstamp << std::endl;
  }

  void Timer::printCurrentTime(const char *msg) {
    std::string tstamp;
    makeTimeStamp( tstamp );
    if (msg) std::cout << msg << std::flush;
    std::cout << tstamp << std::endl;
  }

  void Timer::printUsedTime(const char *msg) {
    time_t loopTime  = getUsedTime();
    int hours,mins,secs;
    hours = static_cast<int>(loopTime/3600);
    mins = (loopTime - hours*3600)/60;
    secs =  loopTime - hours*3600-mins*60;
    if (msg) {
       std::cout << msg << std::flush;
    } else {
       std::cout << "Used time: " << std::flush;
    }
    std::cout << hours << "h " << mins << "m " << secs << "s" << std::endl;
  }

  time_t Timer::calcEstimatedTime(int nloops, int ntotal ) {
    time_t loopTime  = getStopTime() - m_startTime;
    time_t deltaTime = (nloops*loopTime)/ntotal;
    //
    return deltaTime + m_startTime;
  }

  void Timer::printEstimatedTime(int nloops, int ntotal ) {
    time_t esttime = calcEstimatedTime(nloops,ntotal);
    time_t deltaTime = esttime - m_startTime;
    std::string tstamp;
    makeTimeStamp( tstamp, esttime );
    //
    int hours,mins,secs;
    hours = static_cast<int>(deltaTime/3600);
    mins = (deltaTime - hours*3600)/60;
    secs =  deltaTime - hours*3600-mins*60;
    std::cout << "Estimated end of run: " << tstamp;
    std::cout << " ( " << hours << "h " << mins << "m " << secs << "s" << " )\n" << std::endl;
  }

  void Timer::startClock() {
    if (!m_runningClock) {
      m_startClock = clock();
      m_runningClock=true;
    }
  }

  void Timer::stopClock() {
    if (m_runningClock) {
      m_stopClock = clock();
      m_runningClock=false;
    }
  }

  void Timer::printUsedClock(int norm, const char *msg) {
    stopClock();
    double dt = getUsedClock(1e-3); // return time in ms
    if (msg)
      std::cout << msg << dt << std::endl;
    else
      std::cout << "Total CPU time used (ms)     : " << dt << std::endl;
    if (norm>0) std::cout << "Per event CPU time used (ms) : " << (dt/norm) << std::endl;
  }

};
