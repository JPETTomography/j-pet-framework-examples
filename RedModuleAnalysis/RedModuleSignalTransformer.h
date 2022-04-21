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
 *  @file RedModuleSignalTransformer.h
 */

#ifndef REDMODULESIGNALTRANSFORMER_H
#define REDMODULESIGNALTRANSFORMER_H

#include "JPetUserTask/JPetUserTask.h"
#include "Signals/JPetMatrixSignal/JPetMatrixSignal.h"
#include <boost/property_tree/ptree.hpp>
#include <vector>

class JPetWriter;

/**
 * @brief User Task: method merging PM Signals into Matrix Signals.
 */
class RedModuleSignalTransformer : public JPetUserTask
{
public:
  RedModuleSignalTransformer(const char* name);
  virtual ~RedModuleSignalTransformer();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string kMergeSignalsTimeParamKey = "SignalTransformer_MergeSignalsTime_double";
  const std::string kToTHistoUpperLimitParamKey = "ToTHisto_UpperLimit_double";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  const std::string kWLSConfigFileParamKey = "WLSConfigFile_std::string";
  const std::string kWLSSlotIDParamKey = "RedModule_WLSSlotID_int";
  void saveMatrixSignals(const std::vector<JPetMatrixSignal>& mtxSigVec);
  boost::property_tree::ptree fConstansTree;
  boost::property_tree::ptree fWLSConfigTree;
  void initialiseHistograms();
  double fToTHistoUpperLimit = 200000.0;
  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;
  double fMergingTime = 20000.0;
  int fWLSSlotID = -1;
};

#endif /* !REDMODULESIGNALTRANSFORMER_H */
