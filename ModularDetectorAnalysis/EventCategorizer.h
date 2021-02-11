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
 *  @file EventCategorizer.h
 */

#ifndef EVENTCATEGORIZER_H
#define EVENTCATEGORIZER_H

#include <boost/property_tree/ptree.hpp>

#include "EventCategorizerTools.h"
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <JPetUserTask/JPetUserTask.h>
#include <TVector3.h>
#include <map>
#include <vector>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * @brief User Task categorizing Events
 *
 * Task attempts to add types of events to each event. Each category/type
 * has separate method for checking, if current event fulfills set of conditions.
 * These methods are defined in tools class. More than one type can be added to an event.
 * Set of controll histograms are created, unless the user decides not to produce them.
 */
class EventCategorizer : public JPetUserTask
{
public:
  EventCategorizer(const char* name);
  virtual ~EventCategorizer();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string k2gThetaDiffParamKey = "EventCategorizer_2gThetaDiff_double";
  const std::string k2gTimeDiffParamKey = "EventCategorizer_2gTimeDiff_double";

  const std::string kTOTCutAnniMinParamKey = "EventCategorizer_TOT_Cut_Anni_Min_double";
  const std::string kTOTCutAnniMaxParamKey = "EventCategorizer_TOT_Cut_Anni_Max_double";
  const std::string kTOTCutDeexMinParamKey = "EventCategorizer_TOT_Cut_Deex_Min_double";
  const std::string kTOTCutDeexMaxParamKey = "EventCategorizer_TOT_Cut_Deex_Max_double";

  const std::string kSourcePosXParamKey = "EventCategorizer_SourcePos_X_double";
  const std::string kSourcePosYParamKey = "EventCategorizer_SourcePos_Y_double";
  const std::string kSourcePosZParamKey = "EventCategorizer_SourcePos_Z_double";

  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";

  const std::string kScatterTOFTimeDiffParamKey = "Scatter_Categorizer_TOF_TimeDiff_double";
  const std::string kMaxTimeDiffParamKey = "EventCategorizer_MaxTimeDiff_double";

  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";

  void saveEvents(const std::vector<JPetEvent>& event);

  boost::property_tree::ptree fConstansTree;
  double fScatterTOFTimeDiff = 2000.0;
  double fMaxTimeDiff = 15000.0;
  double f2gThetaDiff = 3.0;
  double f2gTimeDiff = 2000.0;
  double fTOTCutAnniMin = 150000.0;
  double fTOTCutAnniMax = 250000.0;
  double fTOTCutDeexMin = 270000.0;
  double fTOTCutDeexMax = 370000.0;
  double fLORPosCutMin = -5.0;
  double fLORPosCutMax = 5.0;
  double fMaxSourceDist = 5.0;
  TVector3 fSourcePos;

  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;

  void initialiseHistograms();
  void initialiseCalibrationHistograms();
};
#endif /* !EVENTCATEGORIZER_H */
