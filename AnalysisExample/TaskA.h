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
 *  @file TaskA.h
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
  virtual void init(const JPetTaskInterface::Options& opts) override;
  virtual void exec();
  virtual void terminate();

  void setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
  }
  const JPetParamBank& getParamBank() {
    assert(fParamManager);
    return fParamManager->getParamBank();
  }
protected:
  JPetParamManager* fParamManager;
  int fCurrEventNumber;
  
};
#endif /*  !TASKA_H */
