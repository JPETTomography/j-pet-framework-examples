/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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

#include <vector>
#include <map>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventFinder : public JPetUserTask{
public:
	EventFinder(const char * name);
	virtual ~EventFinder(){}
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
protected:
  	double kEventTimeWindow = 5000.0; //ps
	const std::string fEventTimeParamKey = "EventFinder_EventTime_float";
    	std::vector<JPetHit> fHitVector;
  	bool fSaveControlHistos = true;
	void saveEvents(const std::vector<JPetEvent>& event);
	std::vector<JPetEvent> buildEvents(const JPetTimeWindow & hits);
};
#endif /*  !EVENTFINDER_H */
