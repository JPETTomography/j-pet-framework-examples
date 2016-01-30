/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskC3.h 
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */ 

#ifndef TASKC3_H 
#define TASKC3_H 

#include "JPetTask/JPetTask.h"
#include "JPetPhysSignal/JPetPhysSignal.h"
#include "JPetRecoSignal/JPetRecoSignal.h"

class JPetWriter;

class TaskC3:public JPetTask {
 public:
  TaskC3(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {fWriter =writer;}
 protected:
  JPetPhysSignal createPhysSignal(JPetRecoSignal& signals);
  void savePhysSignal( JPetPhysSignal signal);
  // for statistics of the processing:
  
  JPetWriter* fWriter;

  //ClassDef(TaskC3, 1);
};
#endif /*  !TASKC3_H */
