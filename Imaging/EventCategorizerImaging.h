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
 *  @file EventCategorizerImaging.h
 */

#ifndef EVENTCATEGORIZERIMAGING_H
#define EVENTCATEGORIZERIMAGING_H

#include <Hits/JPetBaseHit/JPetBaseHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetEventType/JPetEventType.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetUserTask/JPetUserTask.h>
#include <map>
#include <vector>

class JPetWriter;

class EventCategorizerImaging : public JPetUserTask
{
public:
  EventCategorizerImaging(const char* name);
  virtual ~EventCategorizerImaging() {}
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  const std::string k2GammaMaxAngleParamKey = "Imaging_2GammaMaxAngle_double";
  const std::string k3GammaMinAngleParamKey = "Imaging_3GammaMinAngle_double";

  const std::string kMinAnnihTOTParamKey = "Imaging_MinAnnihTOT_double";
  const std::string kMaxAnnihTOTParamKey = "Imaging_MaxAnnihTOT_double";

  const std::string kScatterTimeDiffParamKey = "Imaging_ScatterTimeDiff_double";
  const std::string k2GammaMaxTOFParamKey = "Imaging_2GammaMaxTOF_double";
  const std::string k3GammaMaxTimeDiffParamKey = "Imaging_3GammaMaxTimeDiff_double";

  const std::string kMaxDistOfDecayPlaneFromCenterParamKey = "Imaging_MaxDistOfDecayPlaneFromCenter_double";

  const std::string kSaveLORsOnlyParamKey = "Imaging_SaveLORsOnly_bool";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";

  double f2GammaMaxAngle = 5.0;
  double f3GammaMinAngle = 180.0;

  double fMinAnnihTOT = 150000.0;
  double fMaxAnnihTOT = 250000.0;

  double fScatterTOFTimeDiff = 1000.0;
  double f2GammaMaxTOF = 2000.0;
  double f3GammaMaxTimeDiff = 2000.0;

  double fDPCenterDist = 5.0;

  bool fSaveHistos = true;
  bool fSaveLORsOnly = false;

  void saveEvents(const std::vector<JPetEvent>& event);
  void initialiseHistograms();
};

#endif /* !EVENTCATEGORIZERIMAGING_H */
