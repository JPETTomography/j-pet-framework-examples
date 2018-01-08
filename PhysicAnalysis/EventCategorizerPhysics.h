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
 *  @file EventCategorizerPhysics.h
 */

#ifndef EVENTCATEGORIZERPHYSICS_H
#define EVENTCATEGORIZERPHYSICS_H
#include <vector>
#include <map>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetEventType/JPetEventType.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetStatistics/JPetStatistics.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizerPhysics : public JPetUserTask{
public:  
	EventCategorizerPhysics(const char * name);
	virtual ~EventCategorizerPhysics(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
	std::vector<unsigned> PhysicsAnalysis( std::vector<JPetHit> Hits );

protected:
	
	double kMinAnnihilationTOT = 10000.0;
	double kMaxAnnihilationTOT = 25000.0;
	const std::string fMinAnnihilationParamKey = "EventCategorizer_MinAnnihilationTOT_float";
	const std::string fMaxAnnihilationParamKey = "EventCategorizer_MaxAnnihilationTOT_float";
	
	double kMinDeexcitationTOT = 30000.0;
	double kMaxDeexcitationTOT = 50000.0;
	const std::string fMinDeexcitationParamKey = "EventCategorizer_MinDeexcitationTOT_float";
	const std::string fMaxDeexcitationParamKey = "EventCategorizer_MaxDeexcitationTOT_float";
	
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
};


#endif

