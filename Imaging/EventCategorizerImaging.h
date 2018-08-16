/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
#include <vector>
#include <map>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetEventType/JPetEventType.h>
#include <JPetStatistics/JPetStatistics.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizerImaging : public JPetUserTask{
public:
	EventCategorizerImaging(const char * name);
	virtual ~EventCategorizerImaging(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
	JPetEvent imageReconstruction( std::vector<JPetHit> hits );

protected:
  double fScatterTOFTimeDiff = 2000.0;
	double fMinAnnihilationTOT = 10000.0;
	double fMaxAnnihilationTOT = 25000.0;
	const std::string kMinAnnihilationParamKey = "EventCategorizer_MinAnnihilationTOT_float";
	const std::string kMaxAnnihilationParamKey = "EventCategorizer_MaxAnnihilationTOT_float";
	double fMaxZPos = 23.;
	double fMaxDistOfDecayPlaneFromCenter = 5.;
	double fMaxTimeDiff = 1.;
	double fBackToBackAngleWindow = 3.;
	double fDecayInto3MinAngle = 190.;
	const std::string kMaxZPosParamKey = "EventCategorizer_MaxHitZPos_float";
	const std::string kMaxDistOfDecayPlaneFromCenterParamKey = "EventCategorizer_MaxDistOfDecayPlaneFromCenter_float";
	const std::string kMaxTimeDiffParamKey = "EventCategorizer_MaxTimeDiff_float";
	const std::string kBackToBackAngleWindowParamKey = "EventCategorizer_BackToBackAngleWindow_float";
	const std::string kDecayInto3MinAngleParamKey = "EventCategorizer_DecayInto3MinAngle_float";
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
};


#endif
