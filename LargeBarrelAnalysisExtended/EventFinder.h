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
#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class EventFinder : public JPetTask{
public:
	EventFinder(const char * name, const char * description);
	virtual ~EventFinder(){}
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
protected:
  	double kEventTimeWindow = 5000.0; //ps
	const std::string fEventTimeParamKey = "EventFinder_EventTime";
    	std::vector<JPetHit> fHitVector;
  	bool fSaveControlHistos = true;
	void saveEvents(const std::vector<JPetEvent>& event);
	std::vector<JPetEvent> buildEvents(std::vector<JPetHit> hitVec);
};
#endif /*  !EVENTFINDER_H */
