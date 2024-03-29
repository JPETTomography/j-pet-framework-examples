/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventAnalyzer.h
 */

#ifndef EVENTANALYZER_H
#define EVENTANALYZER_H

#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <JPetUserTask/JPetUserTask.h>
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"

class EventAnalyzer : public JPetUserTask {
public:
  EventAnalyzer(const char *name);
  virtual ~EventAnalyzer();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void fillResolutionHistograms(const JPetEvent &event,
                                const JPetTimeWindowMC *tw);
  bool fIsMC = false;
};
#endif /* !EVENTANALYZER_H */
