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
 *  @file TaskE.h
 */

#ifndef TASKE_H 
#define TASKE_H 

#include "JPetTask/JPetTask.h"
#include "JPetHit/JPetHit.h"
#include "JPetRawSignal/JPetRawSignal.h"

#include "LargeBarrelMapping.h"

class JPetWriter;

class TaskE:public JPetTask {
 public:
  TaskE(const char * name, const char * description);
  virtual void init(const JPetTaskInterface::Options& opts);
  virtual void exec();
  virtual void terminate();
  virtual void setWriter(JPetWriter* writer) {fWriter =writer;}
 protected:
  const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);

  void fillCoincidenceHistos(std::vector<JPetHit>& hits);
  void fillDeltaIDhisto(int delta_ID, int threshold, const JPetLayer & layer);
  void fillTOFvsDeltaIDhisto(int delta_ID, int threshold, const JPetHit & hit1, const JPetHit & hit2);
  bool isGoodTimeDiff(const JPetHit & hit, int thr);
  void fillTOTvsTOThisto(int delta_ID, int thr, const JPetHit & hit1, const JPetHit & hit2);
  
  LargeBarrelMapping fBarrelMap;

  std::vector<JPetHit> fHits;
  
  JPetWriter* fWriter;

  //ClassDef(TaskE, 1);
};
#endif /*  !TASKE_H */
