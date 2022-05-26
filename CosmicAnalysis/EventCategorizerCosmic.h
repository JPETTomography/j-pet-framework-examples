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
 *  @file EventCategorizerCosmic.h
 */

#ifndef EVENTCATEGORIZERCOSMIC_H
#define EVENTCATEGORIZERCOSMIC_H

#include <JPetStatistics/JPetStatistics.h>
#include <JPetEventType/JPetEventType.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <vector>

class JPetWriter;

class EventCategorizerCosmic : public JPetUserTask
{
public:
	EventCategorizerCosmic(const char * name);
	virtual ~EventCategorizerCosmic(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
	JPetEvent cosmicAnalysis(std::vector<JPetHit> hits);

protected:
	const std::string kMinCosmicTOTParamKey = "EventCategorizer_MinCosmicTOT_float";
    const std::string kTOTCalculationType = "HitFinder_TOTCalculationType_std::string";
	void saveEvents(const std::vector<JPetEvent>& event);
	double fMinCosmicTOT = 55000.0;
	bool fSaveControlHistos = true;
    std::string fTOTCalculationType = "";
    void initialiseHistograms();
};

#endif /* !EVENTCATEGORIZERCOSMIC_H */
