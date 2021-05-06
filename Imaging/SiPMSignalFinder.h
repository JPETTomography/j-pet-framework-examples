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
 *  @file SiPMSignalFinder.h
 */

#ifndef SIPMSIGNALFINDER_H
#define SIPMSIGNALFINDER_H

#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetUserTask/JPetUserTask.h>
#include <boost/property_tree/ptree.hpp>
#include <vector>

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
class SiPMSignalFinder : public JPetUserTask
{
public:
  SiPMSignalFinder(const char* name);
  virtual ~SiPMSignalFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveRawSignals(const std::vector<JPetRawSignal>& sigChVec);
  const std::string kLeadTrailMaxTimeParamKey = "SiPMSignalFinder_LeadTrailMaxTime_double";
  const std::string kUseCorruptedSigChParamKey = "SiPMSignalFinder_UseCorruptedSigCh_bool";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kEdgeMaxTimeParamKey = "SiPMSignalFinder_EdgeMaxTime_double";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  boost::property_tree::ptree fConstansTree;
  const int kNumOfThresholds = 2;
  double fSigChLeadTrailMaxTime = 23000.0;
  double fSigChEdgeMaxTime = 5000.0;
  bool fUseCorruptedSigCh = false;
  bool fSaveControlHistos = true;
  double fScalingFactor = 0.001;

  void initialiseHistograms();
};

#endif /* !SIPMSIGNALFINDER_H */
