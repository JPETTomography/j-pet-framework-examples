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
#include <JPetUserTask/JPetUserTask.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * @brief User Task: method organizing Signal Channels to Raw Signals
 *
 * Task organizes Signal Channels from every JPetTimeWindow to Raw Signals
 * Parameters for time window values used in tools can be specified in user options,
 * default are provided.
 */

class SignalFinder: public JPetUserTask
{
public:
  SignalFinder(const char* name);
  virtual ~SignalFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveRawSignals(const std::vector<JPetRawSignal>& sigChVec);
  const std::string kEdgeMaxTimeParamKey = "SignalFinder_EdgeMaxTime_float";
  const std::string kLeadTrailMaxTimeParamKey = "SignalFinder_LeadTrailMaxTime_float";
  const std::string kSaveControlHistosParamKey = "Save_Cotrol_Histograms_bool";
  const int kNumOfThresholds = 4;
  double fSigChLeadTrailMaxTime = 25000.0;
  double fSigChEdgeMaxTime = 5000.0;
  bool fSaveControlHistos = true;
  void initialiseHistograms();
};
#endif
/*  !SIGNALFINDER_H */
