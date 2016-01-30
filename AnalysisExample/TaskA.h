/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskA.h
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */

#ifndef TASKA_H
#define TASKA_H

#include <vector>

#include "JPetTask/JPetTask.h"
#include "JPetTimeWindow/JPetTimeWindow.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetParamManager/JPetParamManager.h"

class JPetWriter;

class TaskA: public JPetTask
{
public:
  TaskA(const char * name, const char * description);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {
    fWriter = writer;
  }
  void setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
  }
  const JPetParamBank& getParamBank() {
    assert(fParamManager);
    return fParamManager->getParamBank();
  }
protected:
  void saveTimeWindow( JPetTimeWindow slot);

  JPetWriter* fWriter;
  JPetParamManager* fParamManager;
  int fCurrEventNumber;

  //ClassDef(TaskA, 1);
  
};
#endif /*  !TASKA_H */
