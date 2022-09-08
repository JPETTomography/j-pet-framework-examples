/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleHitFinder.h
 */

#ifndef REDMODULEHITFINDER_H
#define REDMODULEHITFINDER_H

#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetUserTask/JPetUserTask.h>
#include <Signals/JPetMatrixSignal/JPetMatrixSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <vector>

class JPetWriter;

/**
 * @brief User Task creating JPetPhysRecoHit from matched Singlas
 *
 * Task pairs Matrix Signals and creates Hits, based on time comparison
 * of Signals, time window for hit matching can be specified in user options,
 * default one is provided. Matching method is contained in tools class.
 */
class RedModuleHitFinder : public JPetUserTask
{

public:
  RedModuleHitFinder(const char* name);
  virtual ~RedModuleHitFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveHits(const std::vector<JPetPhysRecoHit>& hits);
  void initialiseHistograms();
  const std::string kMinHitMultiDiffParamKey = "RedModuleHitFinder_MinHitMultiplicity_int";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kToTHistoUpperLimitParamKey = "ToTHisto_UpperLimit_double";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  const std::string kWLSConfigFileParamKey = "WLSConfigFile_std::string";
  const std::string kABTimeDiffParamKey = "RedModuleHitFinder_ABTimeDiff_double";
  const std::string kWLSScinTimeDiffParamKey = "RedModuleHitFinder_WLSScinTimeDiff_double";
  const std::string kTimeDiffOffsetParamKey = "RedModuleHitFinder_TimeDiffOffset_double";
  boost::property_tree::ptree fConstansTree;
  boost::property_tree::ptree fWLSConfigTree;
  double fToTHistoUpperLimit = 200000.0;
  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;
  double fABTimeDiff = 10000.0;
  double fWLSScinTimeDiff = 10000.0;
  double fTimeDiffOffset = 0.0;
  int fMinHitMultiplicity = -1;

  void fillHistograms(const std::vector<JPetPhysRecoHit>& hits);
};

#endif /* !REDMODULEHITFINDER_H */
