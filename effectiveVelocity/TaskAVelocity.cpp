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
 *  @file TaskAVelocityVelocity.cpp
 */
#include <Unpacker2/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "TaskAVelocity.h"
TaskAVelocity::TaskAVelocity(const char * name, const char * description)
:JPetTask(name, description),fCurrEventNumber(0){}
void TaskAVelocity::init(const JPetTaskInterface::Options& opts){
	getStatistics().createHistogram( new TH1F("HitsPerEvtCh","Hits per channel in one event",50,-0.5,49.5) );
	getStatistics().createHistogram( new TH1F("ChannelsPerEvt","Channels fired in one event",200,-0.5,199.5) );
}
TaskAVelocity::~TaskAVelocity(){}
void TaskAVelocity::exec(){  
	//getting the data from event in propriate format
	//const is commented because this class has inproper architecture:
	// all get-methods aren't tagged with const modifier
	if(auto evt = reinterpret_cast</*const*/ EventIII*const>(getEvent())){
		int ntdc = evt->GetTotalNTDCChannels();
		getStatistics().getHisto1D("ChannelsPerEvt").Fill( ntdc );
		JPetTimeWindow tslot;
		tslot.setIndex(fCurrEventNumber);
		auto tdcHits = evt->GetTDCChannelsArray();
		for (int i = 0; i < ntdc; ++i) {
			//const is commented because this class has inproper architecture:
			// all get-methods aren't tagged with const modifier
			auto tdcChannel = dynamic_cast</*const*/ TDCChannel*const>(tdcHits->At(i));
			auto tomb_number =  tdcChannel->GetChannel();
			if (tomb_number % 65 == 0) { // skip trigger signals from TRB
				continue;
			}
			if( getParamBank().getTOMBChannels().count(tomb_number) == 0 ) {
				WARNING(Form("DAQ Channel %d appears in data but does not exist in the setup from DB.", tomb_number));
				continue;
			}
			JPetTOMBChannel& tomb_channel = getParamBank().getTOMBChannel(tomb_number);
			// one TDC channel may record multiple signals in one TSlot
			// iterate over all signals from one TDC channel
			// analyze number of hits per channel
			getStatistics().getHisto1D("HitsPerEvtCh").Fill( tdcChannel->GetHitsNum() );
			// !!!!!! only to imitate the Go4 macros
			// !!!!!! which take only last signals from the event    
			for(int j = tdcChannel->GetHitsNum()-1; j < tdcChannel->GetHitsNum(); ++j){
				JPetSigCh sigChTmpLead, sigChTmpTrail;
				sigChTmpLead.setDAQch(tomb_number);
				sigChTmpTrail.setDAQch(tomb_number);
				sigChTmpLead.setType(JPetSigCh::Leading);
				sigChTmpTrail.setType(JPetSigCh::Trailing);
				sigChTmpLead.setThresholdNumber(tomb_channel.getLocalChannelNumber());
				sigChTmpTrail.setThresholdNumber(tomb_channel.getLocalChannelNumber());
				sigChTmpLead.setPM(tomb_channel.getPM());
				sigChTmpLead.setFEB(tomb_channel.getFEB());
				sigChTmpLead.setTRB(tomb_channel.getTRB());
				sigChTmpLead.setTOMBChannel(tomb_channel);
				sigChTmpTrail.setPM(tomb_channel.getPM());
				sigChTmpTrail.setFEB(tomb_channel.getFEB());
				sigChTmpTrail.setTRB(tomb_channel.getTRB());
				sigChTmpTrail.setTOMBChannel(tomb_channel);
				sigChTmpLead.setThreshold(tomb_channel.getThreshold());
				sigChTmpTrail.setThreshold(tomb_channel.getThreshold());
				// check for empty TDC times
				if( tdcChannel->GetLeadTime(j) == -100000 )continue;
				if( tdcChannel->GetTrailTime(j) == -100000 )continue;
				// finally, set the times in ps [raw times are in ns]
				sigChTmpLead.setValue(tdcChannel->GetLeadTime(j) * 1000.);
				sigChTmpTrail.setValue(tdcChannel->GetTrailTime(j) * 1000.);
				tslot.addCh(sigChTmpLead);
				tslot.addCh(sigChTmpTrail);
			}
		}
		saveTimeWindow(tslot);
		fCurrEventNumber++;
	}
}

void TaskAVelocity::terminate(){}
void TaskAVelocity::saveTimeWindow( JPetTimeWindow slot){
	assert(fWriter);
	fWriter->write(slot);
}
void TaskAVelocity::setWriter(JPetWriter* writer){
	fWriter=writer;
}
void TaskAVelocity::setParamManager(JPetParamManager* paramManager) {
	fParamManager = paramManager;
}
const JPetParamBank& TaskAVelocity::getParamBank()const{
	assert(fParamManager);
	return fParamManager->getParamBank();
}
