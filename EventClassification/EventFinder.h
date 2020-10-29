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
 *  @file EventFinder.h
 */

#ifndef EVENTFINDER_H
#define EVENTFINDER_H

#include <JPetUserTask/JPetUserTask.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <vector>
#include <map>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * @brief User Task creating JPetEvent from hits
 *
 * Simple task, that groups hits into uncategorized Events
 * with the use of some time window value. This value is given by
 * default, but it can be provided by the user in parameters file.
 * Also user can require to save only Events of minimum multiplicity
 * and if include Corrupted Hits in the created events.
 */
class EventFinder: public JPetUserTask
{
public:
  EventFinder(const char * name);
  virtual ~EventFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  std::vector<JPetEvent> buildEvents(const JPetTimeWindow & hits);
  void saveEvents(const std::vector<JPetEvent>& event);
  // std::tuple<int, double, double, double> getStatsPerTHR(const JPetHit& hit, int thrNum);
  // std::tuple<int, double, double, double> getStats(const JPetHit& hit);
  // static double getRawSigBaseTime(const JPetRawSignal& rawSig);
  // void plotOffsetHistograms(const JPetMatrixSignal& mtxSig, std::string side, std::string type);
  void initialiseHistograms();

  const std::string kUseCorruptedHitsParamKey = "EventFinder_UseCorruptedHits_bool";
  const std::string kEventMinMultiplicity = "EventFinder_MinEventMultiplicity_int";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kEventTimeParamKey = "EventFinder_EventTime_double";

  // TOT/multi cut
  // const std::string kToTCutMinParamKey = "EventFinder_ToTCut_Min_double";
  // const std::string kToTCutMaxParamKey = "EventFinder_ToTCut_Max_double";

  // time walk corrections
  // const std::string kTimeWalkCorrAParamKey = "EventFinder_TimeWalk_A_double";
  // const std::string kTimeWalkCorrBParamKey = "EventFinder_TimeWalk_B_double";

  // histogram limits variables
  // const std::string kHistoTDiffMinParamKey = "EventFinder_HistoTDiffMin_double";
  // const std::string kHistoTDiffMaxParamKey = "EventFinder_HistoTDiffMax_double";
  // const std::string kHistoTOTMinParamKey = "EventFinder_HistoTOTOMin_double";
  // const std::string kHistoTOTMaxParamKey = "EventFinder_HistoTOTMax_double";

  // const std::string kZoomTDiffMinParamKey = "EventFinder_ZoomTDiffMin_double";
  // const std::string kZoomTDiffMaxParamKey = "EventFinder_ZoomTDiffMax_double";
  // const std::string kZoomTOTMinParamKey = "EventFinder_ZoomTOTOMin_double";
  // const std::string kZoomTOTMaxParamKey = "EventFinder_ZoomTOTMax_double";

  bool fUseCorruptedHits = false;
  bool fSaveControlHistos = true;
  double fEventTimeWindow = 5000.0;

  // double fTimeWalkAParam = 1.0;
  // double fTimeWalkBParam = 0.0;
  //
  // double fHistoTDiffMin = -10000.0;
  // double fHistoTDiffMax = 10000.0;
  // double fHistoTOTMin = -0.0002;
  // double fHistoTOTMax = 0.0002;
  //
  // double fZoomTDiffMin = -5000.0;
  // double fZoomTDiffMax = 5000.0;
  // double fZoomTOTMin = -0.00001;
  // double fZoomTOTMax = 0.00001;

  unsigned int fMinMultiplicity = 1;
};
#endif /* !EVENTFINDER_H */
