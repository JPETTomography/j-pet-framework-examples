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
 *  @file TaskD.h
 */

#ifndef _____TIME___SYNC____TASK_________ 
#       define _____TIME___SYNC____TASK_________
#include <memory>
#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetLargeBarrelExtensions/PetDict.h>
#include <JPetLargeBarrelExtensions/BarrelExtensions.h>
class Synchronization;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TaskSyncAB:public LargeBarrelTask {
public:
  TaskSyncAB(const char * name, const char * description);
  virtual ~TaskSyncAB();
  virtual void init(const JPetTaskInterface::Options& opts)override;
  virtual void exec()override;
  virtual void terminate()override;
private:
  std::shared_ptr<Synchronization>fSync;
};
#endif
