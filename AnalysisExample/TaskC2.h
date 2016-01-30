/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskC2.h 
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */ 

#ifndef TASKC2_H 
#define TASKC2_H 

#include "JPetTask/JPetTask.h"
#include "JPetRecoSignal/JPetRecoSignal.h"

class JPetWriter;

class TaskC2:public JPetTask {
 public:
  TaskC2(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {fWriter =writer;}
 protected:
  JPetRecoSignal createRecoSignal(JPetRawSignal& rawSignal);
  void saveRecoSignal( JPetRecoSignal signal);
  // for statistics of the processing:
  
  JPetWriter* fWriter;

  //ClassDef(TaskC2, 1);
};
#endif /*  !TASKC2_H */
