/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskB.h
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */

#ifndef TASKB_H
#define TASKB_H

#include <vector>

#include "JPetTask/JPetTask.h"
#include "JPetRawSignal/JPetRawSignal.h"

#include "JPetTimeWindow/JPetTimeWindow.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetParamManager/JPetParamManager.h"

class JPetWriter;

class TaskB: public JPetTask
{
public:
  TaskB(const char * name, const char * description);
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

  void saveTimeWindow( JPetTimeWindow slot);

  JPetWriter* fWriter;
  JPetParamManager* fParamManager;

  //ClassDef(TaskB, 1);
};
#endif /*  !TASKB_H */
