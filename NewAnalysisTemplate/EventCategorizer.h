/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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

#include <vector>
#include <map>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetEventType/JPetEventType.h>
#include "EventCategorizerCosmic.h"
#include "EventCategorizerImaging.h"
#include "EventCategorizerPhysics.h"

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizer : public JPetUserTask{
public:  
	EventCategorizer(const char * name);
	virtual ~EventCategorizer(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;

protected:
	double kMinCosmicTOT = 55000.0;
	const std::string fMinCosmicTOTParamKey = "EventCategorizer_MinCosmicTOT_float";
	bool kCosmicAnalysis = true;
	const std::string fCosmicAnalysisParamKey = "EventCategorizer_CosmicAnalysis_bool";
	
	double kMinAnnihilationTOT = 10000.0;
	double kMaxAnnihilationTOT = 25000.0;
	const std::string fMinAnnihilationParamKey = "EventCategorizer_MinAnnihilationTOT_float";
	const std::string fMaxAnnihilationParamKey = "EventCategorizer_MaxAnnihilationTOT_float";
	bool kImaging = true;
	const std::string fImagingParamKey = "EventCategorizer_Imaging_bool";
	
	double kMinDeexcitationTOT = 30000.0;
	double kMaxDeexcitationTOT = 50000.0;
	const std::string fMinDeexcitationParamKey = "EventCategorizer_MinDeexcitationTOT_float";
	const std::string fMaxDeexcitationParamKey = "EventCategorizer_MaxDeexcitationTOT_float";
	bool kPhysicsAnalysis = true;
	const std::string fPhysicsAnalysisParamKey = "EventCategorizer_PhysicsAnalysis_bool";
	
	EventCategorizerCosmic EventCosmic;
	EventCategorizerImaging EventImaging;
	EventCategorizerPhysics EventPhysics;
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
};

#endif /*  !EVENTCATEGORIZER_H */
