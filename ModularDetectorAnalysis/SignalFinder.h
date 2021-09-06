/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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

#include "SignalFinderTools.h"
#include <JPetUserTask/JPetUserTask.h>
#include <Signals/JPetPMSignal/JPetPMSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <vector>

class JPetWriter;

/**
 * @brief User Task: method organizing Channel Signals to PM Signals
 *
 * Task organizes Signal Channels from every JPetTimeWindow to Raw Signals
 * Parameters for time window values used in tools can be specified in user options,
 * default are provided.
 */
class SignalFinder : public JPetUserTask
{
public:
  SignalFinder(const char* name);
  virtual ~SignalFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void savePMSignals(const std::vector<JPetPMSignal>& pmSigVec);
  const std::string kUseCorruptedChSigParamKey = "SignalFinder_UseCorruptedChSig_bool";
  const std::string kEdgeMaxTimeParamKey = "SignalFinder_EdgeMaxTime_double";
  const std::string kLeadTrailMaxTimeParamKey = "SignalFinder_LeadTrailMaxTime_double";
  const std::string kToTCalculationTypeParamKey = "SignalFinder_ToTCalculationType_std::string";
  const std::string kToTHistoUpperLimitParamKey = "SignalFinder_ToTHistoUpperLimit_double";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  const int kNumOfThresholds = 2;
  boost::property_tree::ptree fConstansTree;
  SignalFinderTools::ToTCalculationType fToTCalcType = SignalFinderTools::kSimplified;
  double fToTHistoUpperLimit = 200000.0;
  double fLeadTrailMaxTime = 200000.0;
  double fEdgeMaxTime = 5000.0;
  double fScalingFactor = 0.0001;
  bool fUseCorruptedChannelSignals = false;
  bool fSaveControlHistos = true;
  void initialiseHistograms();
};

#endif /* !SIGNALFINDER_H */
