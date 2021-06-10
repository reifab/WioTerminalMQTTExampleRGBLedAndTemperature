/*Für zyklische Wiederholung von Prozessen - rollover sicher*/
#ifndef TimePeriod_h
#define TimePeriod_h
#include <Arduino.h>

class TimeInvervall
{
  private:
  unsigned long time_now = 0;
  unsigned int period = 0;
    
  public:
  TimeInvervall(unsigned int periodInMillis){
      period = periodInMillis;
  }

  void setTimeIntervall(unsigned int periodInMillis){
      period = periodInMillis;
  }
  
  bool isTimeElapsed(){
    unsigned long timeDif;
    timeDif = (unsigned long)(millis( ) - time_now);
    if(timeDif > period){
      time_now = millis(); //Misst die aktuelle Zeit seit Systemstart. 
      return true;
    }else{
      return false;
    }
  }
};

#endif