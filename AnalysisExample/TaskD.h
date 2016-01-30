/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskD.h 
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */ 

#ifndef TASKD_H 
#define TASKD_H 

#include "JPetTask/JPetTask.h"
#include "JPetHit/JPetHit.h"
#include "JPetPhysSignal/JPetPhysSignal.h"

class JPetWriter;

class TaskD:public JPetTask {
 public:
  TaskD(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {fWriter =writer;}
 protected:
  std::vector<JPetHit> createHits(std::vector<JPetPhysSignal>& signals);
  void saveHits(std::vector<JPetHit> hits);
  // for statistics of the processing:
  int fInitialSignals;
  int fPairsFound;
  std::vector<JPetPhysSignal> fSignals;
  
  JPetWriter* fWriter;

  //ClassDef(TaskD, 1);
};
#endif /*  !TASKD_H */
