/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskC1.h
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */

#ifndef TASKC1_H
#define TASKC1_H

#include <vector>

#include "JPetTask/JPetTask.h"
#include "JPetRawSignal/JPetRawSignal.h"
#include "JPetTimeWindow/JPetTimeWindow.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetParamManager/JPetParamManager.h"

class JPetWriter;

class TaskC1: public JPetTask
{
public:
  TaskC1(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {
    fWriter = writer;
  }
  void setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
  }
  const JPetParamBank& getParamBank() {
    return fParamManager->getParamBank();
  }
protected:
  void saveRawSignal( JPetRawSignal sig);

  JPetWriter* fWriter;
  JPetParamManager* fParamManager;

  //ClassDef(TaskC1, 1);
};
#endif /*  !TASKC1_H */
