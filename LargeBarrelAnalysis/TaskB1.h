/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file TaskB.h
 */

#ifndef TASKB1_H
#define TASKB1_H

#include <vector>

#include "JPetTask/JPetTask.h"
#include "JPetRawSignal/JPetRawSignal.h"
#include "JPetTimeWindow/JPetTimeWindow.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetParamManager/JPetParamManager.h"

#include "LargeBarrelMapping.h"

class JPetWriter;

class TaskB1: public JPetTask
{
public:
  TaskB1(const char * name, const char * description);
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
  
  const char * formatUniqueChannelDescription(const JPetTOMBChannel & channel, const char * prefix) const;
  int calcGlobalPMTNumber(const JPetPM & pmt) const;
  
  JPetWriter* fWriter;
  JPetParamManager* fParamManager;
  
  LargeBarrelMapping fBarrelMap;
  
  //ClassDef(TaskB1, 1);
};
#endif /*  !TASKB1_H */
