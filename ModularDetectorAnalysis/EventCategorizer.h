/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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

#include "EventCategorizerTools.h"
#include <JPetEvent/JPetEvent.h>
#include <JPetUserTask/JPetUserTask.h>
#include <TVector3.h>
#include <boost/property_tree/ptree.hpp>
#include <map>
#include <vector>

class JPetWriter;

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
  const std::string kEventTimeParamKey = "EventFinder_EventTime_double";

  const std::string k2gThetaDiffParamKey = "EventCategorizer_2gThetaDiff_double";
  const std::string k2gTimeDiffParamKey = "EventCategorizer_2gTimeDiff_double";

  const std::string k3gMinRelAngleParamKey = "EventCategorizer_3gMinRelativeAngle_double";

  const std::string kToTCutAnniMinParamKey = "EventCategorizer_ToT_Cut_Anni_Min_double";
  const std::string kToTCutAnniMaxParamKey = "EventCategorizer_ToT_Cut_Anni_Max_double";
  const std::string kToTCutDeexMinParamKey = "EventCategorizer_ToT_Cut_Deex_Min_double";
  const std::string kToTCutDeexMaxParamKey = "EventCategorizer_ToT_Cut_Deex_Max_double";

  const std::string kToTHistoUpperLimitParamKey = "ToTHisto_UpperLimit_double";

  const std::string kSourcePosXParamKey = "EventCategorizer_SourcePos_X_double";
  const std::string kSourcePosYParamKey = "EventCategorizer_SourcePos_Y_double";
  const std::string kSourcePosZParamKey = "EventCategorizer_SourcePos_Z_double";

  const std::string kSourceDistCutXYParamKey = "EventCategorizer_SourceDist_Cut_XY_double";
  const std::string kSourceDistCutZParamKey = "EventCategorizer_SourceDist_Cut_Z_double";

  const std::string kLORAngleCutParamKey = "EventCategorizer_LORAngle_Cut_double";
  const std::string kLORPosZCutParamKey = "EventCategorizer_LORPosZ_Cut_double";

  const std::string kConstantsFileParamKey = "ConstantsFile_std::string";

  const std::string kScatterTOFTimeDiffParamKey = "Scatter_Categorizer_TOF_TimeDiff_double";
  const std::string kScatterTestTypeParamKey = "Scatter_Categorizer_TestType_std::string";

  const std::string kScatterTimeMinParamKey = "Scatter_Time_Min_double";
  const std::string kScatterTimeMaxParamKey = "Scatter_Time_Max_double";

  const std::string kScatterAngleMinParamKey = "Scatter_Angle_Min_double";
  const std::string kScatterAngleMaxParamKey = "Scatter_Angle_Max_double";

  const std::string kMaxTimeDiffParamKey = "EventCategorizer_MaxTimeDiff_double";
  const std::string kDetectorYRotation = "CalibrationTools_Detector_Y_Rotation_Degrees_double";
  const std::string kCosmicMaxThetaDeg = "CalibrationTools_Cosmic_MatTheta_Degrees_double";

  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
  const std::string kTrentoCalibrationParamKey = "Trento_Calibration_bool";

  void saveEvents(const std::vector<JPetEvent>& event);

  boost::property_tree::ptree fConstansTree;
  double fEventTimeWindow = 5000.0;
  double fScatterTOFTimeDiff = 2000.0;
  double fScatterTimeMin = -5000.0;
  double fScatterTimeMax = 0.0;
  double fScatterAngleMin = 160.0;
  double fScatterAngleMax = 180.0;
  double fMaxTimeDiff = 15000.0;
  double f2gThetaDiff = 3.0;
  double f2gTimeDiff = 2000.0;
  double f3gMinRelAngle = 185.0;
  double fToTCutAnniMin = 150000.0;
  double fToTCutAnniMax = 250000.0;
  double fToTCutDeexMin = 270000.0;
  double fToTCutDeexMax = 370000.0;
  double fToTHistoUpperLimit = 200000.0;
  double fLORAngleCut = 5.0;
  double fLORPosZCut = 5.0;
  double fSourceDistXYCut = 5.0;
  double fSourceDistZCut = 10.0;
  double fDetectorYRotationDeg = 60.0;
  double fCosmicMaxThetaDiffDeg = 3.0;
  TVector3 fSourcePos;
  EventCategorizerTools::ScatterTestType fTestType = EventCategorizerTools::kSimpleParam;

  bool fSaveControlHistos = true;
  bool fSaveCalibHistos = false;
  bool fTrentoCalibHistos = false;

  void initialiseHistograms();
  void initialiseCalibrationHistograms(bool includeTrento);
};
#endif /* !EVENTCATEGORIZER_H */
