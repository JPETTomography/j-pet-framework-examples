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
 *  @file TimeWindowCreator.h
 */

#ifndef TIMEWINDOWCREATOR_H
#define TIMEWINDOWCREATOR_H

#include <JPetChannel/JPetChannel.h>
#include <JPetSigCh/JPetSigCh.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetUserTask/JPetUserTask.h>

#include <boost/property_tree/ptree.hpp>

#include <map>
#include <set>

class JPetWriter;

/**
 * @brief User Task: translate Unpacker EventIII data to JPetTimeWindow
 *
 * Task translates data from Unpacker file fomrat - EventIII to JPetTimeWindow.
 * Parameters for start and end time can be specified in user options, default are
 * provided. Moreover time calibration and threshold values injection can be
 * performed, if ASCII files of standard format were provided. In case of errors,
 * creation of Time Windows continues without this additional information.
 */
class TimeWindowCreator : public JPetUserTask
{
public:
  TimeWindowCreator(const char* name);
  virtual ~TimeWindowCreator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveSigChs(const std::vector<JPetSigCh>& sigChVec);
  void initialiseHistograms();
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kMaskedChannlesParamKey = "TimeWindowCreator_MaskedChannels_std::vector<int>";
  const std::string kMaxTimeParamKey = "TimeWindowCreator_MaxTime_double";
  const std::string kMinTimeParamKey = "TimeWindowCreator_MinTime_double";
  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
  boost::property_tree::ptree fConstansTree;
  std::vector<int> fMaskedChannels;
  bool fSaveControlHistos = true;
  double fMinTime = 0.0;
  double fMaxTime = 1.e6;
  // Number for scaling some histograms, so they are not reaching their memory capacity
  double fScalingFactor = 0.001;
};

#endif /* !TIMEWINDOWCREATOR_H */
