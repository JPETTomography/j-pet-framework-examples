/**
 *  @copyright Copyright (c) 2014, Wojciech Krzemien
 *  @file TaskE.h 
 *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
 *  @brief
 */ 

#ifndef TASKE_H 
#define TASKE_H 

#include "JPetTask/JPetTask.h"
#include "JPetHit/JPetHit.h"
#include "JPetLOR/JPetLOR.h"

class JPetWriter;

class TaskE:public JPetTask {
 public:
  TaskE(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {fWriter =writer;}
 protected:
  std::vector<JPetLOR> createLORs(std::vector<JPetHit>& hits);
  void saveLORs(std::vector<JPetLOR> lors);
  // for statistics of the processing:
  int fInitialHits;
  int fPairsFound;
  std::vector<JPetHit> fHits;
  
  JPetWriter* fWriter;

  //ClassDef(TaskE, 1);
};
#endif /*  !TASKE_H */
