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
 *  @file HitFinder.h
 */

#ifndef HITFINDER_H
#define HITFINDER_H

#include "ToTEnergyConverterFactory.h"
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetUserTask/JPetUserTask.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <vector>

class JPetWriter;

/**
 * @brief User Task creating JPetHit from matched Singlas
 *
 * Task pairs Physical Signals and creates Hits, based on time comparison
 * of Signals, time window for hit matching can be specified in user options,
 * default one is provided. Matching method is contained in tools class.
 */
class HitFinder : public JPetUserTask
{

public:
  HitFinder(const char* name);
  virtual ~HitFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveHits(const std::vector<JPetHit>& hits);
  void initialiseHistograms();
  std::map<unsigned int, std::vector<double>> fVelocities;
  const std::string kUseCorruptedSignalsParamKey = "HitFinder_UseCorruptedSignals_bool";
  const std::string kVelocityFileParamKey = "HitFinder_VelocityFile_std::string";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kRefDetScinIDParamKey = "HitFinder_RefDetScinID_int";
  const std::string kABTimeDiffParamKey = "HitFinder_ABTimeDiff_float";
  const std::string kConvertToTParamKey = "HitFinder_ConvertToT_bool";
  const std::string kTOTCalculationType = "HitFinder_TOTCalculationType_std::string";
  const std::string kUseToTSyncParamKey = "HitFinder_SyncToT_bool";
  const std::string kTOTConstantsFileParamKey = "TOTConstantsFile_std::string";
  ToTEnergyConverterFactory fToTConverterFactory;
  bool fUseCorruptedSignals = false;
  bool fSaveControlHistos = true;
  bool fConvertToT = false;
  double fABTimeDiff = 6000.0;
  int fRefDetScinID = -1;
  std::string fTOTCalculationType = "";
  bool fSyncToT = false;
  boost::property_tree::ptree fConstantsTree;
};

#endif /* !HITFINDER_H */
