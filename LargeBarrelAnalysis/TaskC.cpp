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
 *  @file TaskC.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "TaskC.h"
using namespace std;
TaskC::TaskC(const char * name, const char * description):JPetTask(name, description){}
TaskC::~TaskC(){}
void TaskC::init(const JPetTaskInterface::Options& opts){}
void TaskC::exec(){
	if(auto currSignal = dynamic_cast<const JPetRawSignal*const>(getEvent())){
		getStatistics().getCounter("No. initial signals")++;
		if (fSignals.empty()) {
			fSignals.push_back(*currSignal);
		} else {
			if (fSignals[0].getTimeWindowIndex() == currSignal->getTimeWindowIndex()) {
				fSignals.push_back(*currSignal);
			} else {
				saveHits(createHits(fSignals));
				fSignals.clear();
				fSignals.push_back(*currSignal);
			}
		}
	}
}
vector<JPetHit> TaskC::createHits(const vector<JPetRawSignal>&signals){
	vector<JPetHit> hits;
	for (auto i = signals.begin(); i != signals.end(); ++i) {
		for (auto j = i; ++j != signals.end();) {
			if (i->getPM().getScin() == j->getPM().getScin()) {
				JPetRecoSignal recoSignalA;
				JPetRecoSignal recoSignalB;
				JPetPhysSignal physSignalA;
				JPetPhysSignal physSignalB;
				// assign sides A and B properly
				if( 
					(i->getPM().getSide() == JPetPM::SideA)
					&&(j->getPM().getSide() == JPetPM::SideB)
				){
					recoSignalA.setRawSignal(*i);
					recoSignalB.setRawSignal(*j);
				} else if(
					(j->getPM().getSide() == JPetPM::SideA)
					&&(i->getPM().getSide() == JPetPM::SideB)
				){
					recoSignalA.setRawSignal(*j);
					recoSignalB.setRawSignal(*i);
				} else {
					// if two hits on the same side, ignore
					WARNING("TWO hits on the same scintillator side we ignore it");         
					continue;
				}
				physSignalA.setRecoSignal(recoSignalA);
				physSignalB.setRecoSignal(recoSignalB);
				JPetHit hit;
				hit.setSignalA(physSignalA);
				hit.setSignalB(physSignalB);
				hit.setScintillator(i->getPM().getScin());
				hit.setBarrelSlot(i->getPM().getScin().getBarrelSlot());
				hits.push_back(hit);
				getStatistics().getCounter("No. found hits")++;
			}
		}
	}
	return hits;
}

void TaskC::terminate(){
	saveHits(createHits(fSignals)); //if there is something left
	INFO( Form("From %d initial signals %d hits were paired.", 
		   static_cast<int>(getStatistics().getCounter("No. initial signals")),
		   static_cast<int>(getStatistics().getCounter("No. found hits")) )
	);
}


void TaskC::saveHits(const vector<JPetHit>&hits){
	assert(fWriter);
	for (auto hit : hits)
		fWriter->write(hit);
}
void TaskC::setWriter(JPetWriter* writer){fWriter =writer;}
