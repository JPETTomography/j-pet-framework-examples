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
 *  @file EventFinder.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "EventFinder.h"

using namespace std;

EventFinder::EventFinder(const char * name, const char * description):JPetTask(name, description){}

void EventFinder::initTimeWindows(){

	//time in ps
	fEventTimeWindows.push_back(5000.0);
	fEventTimeWindows.push_back(10000.0);
	fEventTimeWindows.push_back(20000.0);
	fEventTimeWindows.push_back(30000.0);
	fEventTimeWindows.push_back(40000.0);

}

void EventFinder::init(const JPetTaskInterface::Options&){

	initTimeWindows();
	INFO("Event finding started.");

	if (fSaveControlHistos) {
		for(auto timeWindow : fEventTimeWindows){
			getStatistics().createHistogram(
				new TH1F(Form("hits_per_event_%d", timeWindow),
					Form("Number of Hits in Event %d ps", timeWindow),
					20, 0.5, 20.5));
		}
	}

	INFO("Mapping Hits by Time Window.");
}

void EventFinder::exec(){

	if(auto hit = dynamic_cast<const JPetHit*const>(getEvent())){
		if(hit->isSignalASet() && hit->isSignalBSet()){
			if(hit->getSignalA().getTimeWindowIndex() == hit->getSignalB().getTimeWindowIndex()){
				int timeWindowIndex = hit->getSignalA().getTimeWindowIndex();
				auto search = fHitTimeWindowMap.find(timeWindowIndex);
				if (search == fHitTimeWindowMap.end()) {
					vector<JPetHit> tmp;
					tmp.push_back(*hit);
					fHitTimeWindowMap.insert(pair<int, vector<JPetHit>>(timeWindowIndex, tmp));
				} else {
					search->second.push_back(*hit);
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
	
	vector<JPetEvent> eventVec;

	INFO("Hit mapping by TimeWindow ended. Starting Event Finding");
	for(auto element : fHitTimeWindowMap){
		for(auto timeWindow : fEventTimeWindows){

			vector<JPetHit> hitVec = element.second;
			sort(hitVec.begin(), hitVec.end(), sortByTimeValue);

			while(hitVec.size()>0){
				
				JPetEvent event;
				event.setEventType(JPetEventType::kUnknown);
	
				event.addHit(hitVec.at(0));
				while(hitVec.size()>1){
					if(fabs(hitVec.at(1).getTime()-hitVec.at(0).getTime()) < timeWindow) {
						event.addHit(hitVec.at(1));
						hitVec.erase(hitVec.begin() + 1);
					} else {
						break;
					}
				}
				hitVec.erase(hitVec.begin() + 0);
				if (fSaveControlHistos) getStatistics()
						.getHisto1D(Form("hits_per_event_%d", timeWindow))
						.Fill(event.getHits().size());
				eventVec.push_back(event);
			}

		}
		
	}

	saveEvents(eventVec);
	INFO("Event finding ended. Writing to tree");

}

void EventFinder::setWriter(JPetWriter* writer) { fWriter = writer; }

void EventFinder::saveEvents(const vector<JPetEvent>& events)
{
  assert(fWriter);
  for (const auto & event : events) {
    fWriter->write(event);
  }
}
