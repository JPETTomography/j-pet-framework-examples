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
 *  @file TaskC2.h
 */

#ifndef TASKC2_H
#define TASKC2_H

#include "JPetTask/JPetTask.h"
#include "JPetRecoSignal/JPetRecoSignal.h"

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

class JPetWriter;

class TaskC2: public JPetTask
{
public:
  TaskC2(const char* name, const char* description);
  virtual void init(const JPetTaskInterface::Options& opts)override;
  virtual void exec()override;
  virtual void terminate()override;
  virtual void setWriter(JPetWriter* writer) override{
    fWriter = writer;
  }
protected:
  JPetRecoSignal createRecoSignal(JPetRawSignal& rawSignal);
  JPetPhysSignal createPhysSignal(JPetRecoSignal& signals);
  void savePhysSignal( JPetPhysSignal signal);

  JPetWriter* fWriter;

  //ClassDef(TaskC2, 1);
};
#endif /*  !TASKC2_H */
