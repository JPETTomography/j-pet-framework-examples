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
 *  @file SigMatLoader.h
 */

#ifndef SIGMATLOADER_H
#define SIGMATLOADER_H

#include "JPetUserTask/JPetUserTask.h"
#include "Signals/JPetMatrixSignal/JPetMatrixSignal.h"
#include <JPetMatrix/JPetMatrix.h>
#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <unordered_map>
#include <vector>

class JPetWriter;

/**
 * @brief User Task: ...
 */
class SigMatLoader : public JPetUserTask {
public:
  SigMatLoader(const char *name);
  virtual ~SigMatLoader();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string kMergeSignalsTimeParamKey =
      "SigMatLoader_MergeSignalsTime_double";
  const std::string kToTHistoUpperLimitParamKey = "ToTHisto_UpperLimit_double";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  void saveMatrixSignals(const std::vector<JPetMatrixSignal> &mtxSigVec);
  boost::property_tree::ptree fConstansTree;
  void initialiseHistograms();
  double fToTHistoUpperLimit = 200000.0;
  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;
  double fMergingTime = 20000.0;
  std::unordered_map<std::uint32_t, std::unordered_map<int, JPetMatrix *>>
      fMatrixMapping;
  std::unordered_map<int, std::unordered_map<int, JPetPM *>> fPMMapping;
};

#endif /* !SIGMATLOADER_H */
