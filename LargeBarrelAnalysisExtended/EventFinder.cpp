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
 *  @file EventFinder.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "EventFinder.h"

using namespace std;

EventFinder::EventFinder(const char * name, const char * description):JPetTask(name, description){}

void EventFinder::init(const JPetTaskInterface::Options& opts){

	INFO("Event finding started.");

	if (opts.count(fEventTimeParamKey))
		kEventTimeWindow = std::atof(opts.at(fEventTimeParamKey).c_str());

	if (fSaveControlHistos)
		getStatistics().createHistogram(
			new TH1F("hits_per_event","Number of Hits in Event",20, 0.5, 20.5)
		);

}

void EventFinder::exec(){

	if(auto hit = dynamic_cast<const JPetHit*const>(getEvent())){
		if(hit->isSignalASet() && hit->isSignalBSet()){
			if(hit->getSignalA().getTimeWindowIndex() == hit->getSignalB().getTimeWindowIndex()){
				if(kFirstTime){
					kTimeSlotIndex = hit->getSignalA().getTimeWindowIndex();
					fHitVector.push_back(*hit);
					kFirstTime = false;
				}else{
					if(kTimeSlotIndex == hit->getSignalA().getTimeWindowIndex()){
						fHitVector.push_back(*hit);
					}else{
						vector<JPetEvent> events = buildEvents(fHitVector);
						saveEvents(events);
						fHitVector.clear();
						kTimeSlotIndex = hit->getSignalA().getTimeWindowIndex();
						fHitVector.push_back(*hit);
					}
				}
			}
		}
	}
}

//sorting method
bool sortByTimeValue(JPetHit hit1, JPetHit hit2) {
	return (hit1.getTime() < hit2.getTime());
}

void EventFinder::terminate(){
	INFO("Event fiding ended.");
}

vector<JPetEvent> EventFinder::buildEvents(vector<JPetHit> hitVec){

	vector<JPetEvent> eventVec;
	sort(hitVec.begin(), hitVec.end(), sortByTimeValue);

	while(hitVec.size()>0){

		JPetEvent event;
		event.setEventType(JPetEventType::kUnknown);

		event.addHit(hitVec.at(0));

		while(hitVec.size()>1){
			if(fabs(hitVec.at(1).getTime()-hitVec.at(0).getTime()) < kEventTimeWindow) {
				event.addHit(hitVec.at(1));
				hitVec.erase(hitVec.begin() + 1);
			} else {
				break;
			}
		}

		hitVec.erase(hitVec.begin() + 0);

		if (fSaveControlHistos) getStatistics()
														.getHisto1D("hits_per_event")
														.Fill(event.getHits().size());

		eventVec.push_back(event);
	}

	return eventVec;
}

void EventFinder::setWriter(JPetWriter* writer) { fWriter = writer; }

void EventFinder::saveEvents(const vector<JPetEvent>& events)
{
  assert(fWriter);
  for (const auto & event : events) {
    fWriter->write(event);
  }
}
