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

#ifndef HITFINDER1_H
#define HITFINDER1_H

#include <boost/property_tree/ptree.hpp>

#include <JPetMatrixSignal/JPetMatrixSignal.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <vector>
#include <map>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * @brief User Task creating JPetHit from matched Singlas
 *
 * Task pairs Matrix Signals and creates Hits, based on time comparison
 * of Signals, time window for hit matching can be specified in user options,
 * default one is provided. Matching method is contained in tools class.
 */
class HitFinder: public JPetUserTask {

public:
  HitFinder(const char* name);
  virtual ~HitFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveHits(const std::vector<JPetHit>& hits);
  void initialiseHistograms();
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  const std::string kABTimeDiffParamKey = "HitFinder_ABTimeDiff_double";
  boost::property_tree::ptree fConstansTree;
  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;
  double fABTimeDiff = 10000.0;
};

#endif /* !HITFINDER1_H */
