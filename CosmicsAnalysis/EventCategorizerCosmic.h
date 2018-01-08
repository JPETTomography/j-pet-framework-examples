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
 *  @file EventCategorizerCosmic.h
 */

#ifndef EVENTCATEGORIZERCOSMIC_H
#define EVENTCATEGORIZERCOSMIC_H

#include <vector>
#include <JPetHit/JPetHit.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetEventType/JPetEventType.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventCategorizerCosmic : public JPetUserTask{
public:  
	EventCategorizerCosmic(const char * name);
	virtual ~EventCategorizerCosmic(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
	int CosmicAnalysis( std::vector<JPetHit> Hits );

protected:
	double kMinCosmicTOT = 55000.0;
	const std::string fMinCosmicTOTParamKey = "EventCategorizer_MinCosmicTOT_float";
	
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
};

#endif /*  !EVENTCATEGORIZERCOSMIC_H */
