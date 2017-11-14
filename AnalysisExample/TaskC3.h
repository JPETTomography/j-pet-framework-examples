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
 *  @file TaskC3.h
 */

#ifndef TASKC3_H 
#define TASKC3_H 

#include "JPetTask/JPetTask.h"
#include "JPetPhysSignal/JPetPhysSignal.h"
#include "JPetRecoSignal/JPetRecoSignal.h"

class TaskC3:public JPetTask {
 public:
  TaskC3(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
 protected:
  JPetPhysSignal createPhysSignal(JPetRecoSignal signals);
};
#endif /*  !TASKC3_H */
