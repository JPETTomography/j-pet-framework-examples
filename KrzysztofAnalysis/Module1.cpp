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
 *  @file Module1.cpp
 */

#include <Unpacker2/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "Module1.h"

Module1::Module1(const char * name, const char * description) : JPetTask(name, description),fCurrEventNumber(0) {}

Module1::~Module1() {}

void Module1::init(const JPetTaskInterface::Options& ){
	getStatistics().createHistogram( new TH1F("HitsPerEvtCh","Hits per channel in one event",50,-0.5,49.5) );
	getStatistics().createHistogram( new TH1F("ChannelsPerEvt","Channels fired in one event",200,-0.5,199.5) );
}

void Module1::exec(){ 

	//getting event and checking in not null 
	if(auto evt = dynamic_cast<EventIII* const>(getEvent())){

		//get number of TDC channels in event
		int ntdc = evt->GetTotalNTDCChannels();
		getStatistics().getHisto1D("ChannelsPerEvt").Fill( ntdc );

		//creating time window object
		JPetTimeWindow tslot;
		tslot.setIndex(fCurrEventNumber);

		//getting array of TDC channels in event
		auto tdcHits = evt->GetTDCChannelsArray();

		//loop over all TDC channels
		for(int i = 0; i < ntdc; ++i) {
			
			//get current entry from array
			auto tdcChannel = dynamic_cast<TDCChannel*const> (tdcHits->At(i));
			auto tomb_number =  tdcChannel->GetChannel();

			//skip trigger signals from TRB - every 65th signal
			if(tomb_number % 65 == 0) continue;

			//check if there is something in DB
			if(getParamBank().getTOMBChannels().count(tomb_number) == 0) {
				WARNING(Form("DAQ Channel %d appears in data but does not exist in the setup from DB.", tomb_number));
				continue;
			}

			//get TOMB channel from Param bank
			JPetTOMBChannel& tomb_channel = getParamBank().getTOMBChannel(tomb_number);

			// analyze number of hits per channel
			getStatistics().getHisto1D("HitsPerEvtCh").Fill( tdcChannel->GetHitsNum() );

			// one TDC channel may record multiple signals in one Time Window
			// iterate over all signals from one TDC channel
			const int kNumHits = tdcChannel->GetHitsNum();

			//loop over all Channels in current Time Window
			for(int j = 0; j < kNumHits; ++j){

				// check for unreasable times
				// the times should be negative (measured w.r.t end of time window)
				// and not smaller than -1*timeWindowWidth (which can vary for different)
				// data but shoudl not exceed 1 ms, i.e. 1.e6 ns)
				if(tdcChannel->GetLeadTime(j) > kMaxTime ||
				   tdcChannel->GetLeadTime(j) < kMinTime ) continue;
				if(tdcChannel->GetTrailTime(j) > kMaxTime ||
        			   tdcChannel->GetTrailTime(j) < kMinTime ) continue;

				//create Signal Channel objects for 
				JPetSigCh sigChTmpLead = generateSigCh(tomb_channel, JPetSigCh::Leading);
				JPetSigCh sigChTmpTrail = generateSigCh(tomb_channel, JPetSigCh::Trailing);

				//set times of signal channels in ps [raw times are in ns]
				sigChTmpLead.setValue(tdcChannel->GetLeadTime(j) * 1000.);
				sigChTmpTrail.setValue(tdcChannel->GetTrailTime(j) * 1000.);

				//add created signal channels to time window
				tslot.addCh(sigChTmpLead);
				tslot.addCh(sigChTmpTrail);
			}
		}

		//save time window with fWriter
		saveTimeWindow(tslot);

		//iterate event number
		fCurrEventNumber++;
	}
}

void Module1::terminate(){}

void Module1::saveTimeWindow(JPetTimeWindow slot){
	assert(fWriter);
	fWriter->write(slot);
}
void Module1::setWriter(JPetWriter* writer){
	fWriter=writer;
}
void Module1::setParamManager(JPetParamManager* paramManager) {
	fParamManager = paramManager;
}
const JPetParamBank& Module1::getParamBank()const{
	assert(fParamManager);
	return fParamManager->getParamBank();
}

//method setting all info to newly cereated Signal Channel
JPetSigCh Module1::generateSigCh(const JPetTOMBChannel &channel, JPetSigCh::EdgeType edge) const{

	JPetSigCh sigch;
	sigch.setDAQch(channel.getChannel());
	sigch.setType(edge);
	sigch.setThresholdNumber(channel.getLocalChannelNumber());
	sigch.setThreshold(channel.getThreshold());
	sigch.setPM(channel.getPM());
	sigch.setFEB(channel.getFEB());
	sigch.setTRB(channel.getTRB());
	sigch.setTOMBChannel(channel);

	return sigch;
}
