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
 *  @file SignalFinder.h
 */

#ifndef SIGNALFINDER_H
#define SIGNALFINDER_H

#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

class SignalFinder: public JPetTask
{
public:
  SignalFinder(const char* name, const char* description, bool printStats);
  virtual ~SignalFinder();
  virtual void init(const JPetTaskInterface::Options& opts) override;
  virtual void exec() override;
  virtual void terminate() override;
  bool fSaveControlHistos = true;

protected:
  void saveRawSignals(const std::vector<JPetRawSignal>& sigChVec);
  const std::string fEdgeMaxTimeParamKey = "SignalFinder_EdgeMaxTime"; 
  const std::string fLeadTrailMaxTimeParamKey = "SignalFinder_LeadTrailMaxTime";
  Float_t kSigChEdgeMaxTime = 20000; //[ps]
  Float_t kSigChLeadTrailMaxTime = 300000; //[ps]
  const int kNumOfThresholds = 4;
};
#endif
/*  !SIGNALFINDER_H */
