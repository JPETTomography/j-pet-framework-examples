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
 *  @file TimeWindowCreator.cpp
 */
#include <Unpacker2/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "TimeWindowCreator.h"

TimeWindowCreator::TimeWindowCreator(const char* name, const char* description):
  JPetTask(name, description) {}

void TimeWindowCreator::init(const JPetTaskInterface::Options& opts)
{
  fOutputEvents = new JPetTimeWindow("JPetSigCh");
  
  /// Reading values from the user options if available
  if (opts.count(kMaxTimeParamKey)) {
    fMaxTime = std::atof(opts.at(kMaxTimeParamKey).c_str());
  }
  if (opts.count(kMinTimeParamKey)) {
    fMinTime = std::atof(opts.at(kMinTimeParamKey).c_str());
  }
  /*getStatistics().createHistogram( new TH1F("HitsPerEvtCh", "Hits per channel in one event", 50, -0.5, 49.5) );
  getStatistics().createHistogram( new TH1F("ChannelsPerEvt", "Channels fired in one event", 200, -0.5, 199.5) );*/
      getStatistics().createHistogram( new TH1F("ThresholdMultiplicity_1_lead", "First threshold multiplicity on leading edge", 400, 0, 400) );
      getStatistics().createHistogram( new TH1F("ThresholdMultiplicity_2_lead", "Second threshold multiplicity on leading edge", 400, 0, 400) );
      getStatistics().createHistogram( new TH1F("ThresholdMultiplicity_3_lead", "Third threshold multiplicity on leading edge", 400, 0, 400) );
      getStatistics().createHistogram( new TH1F("ThresholdMultiplicity_4_lead", "Fourth threshold multiplicity on leading edge", 400, 0, 400) );
}

TimeWindowCreator::~TimeWindowCreator() {}

void TimeWindowCreator::exec()
{
  //getting the data from event in apropriate format
  //const is commented because this class has inproper architecture:
  // all get-methods aren't tagged with const modifier
  if (auto evt = dynamic_cast </*const*/ EventIII * const > (getEvent())) {
    int ntdc = evt->GetTotalNTDCChannels();
    //getStatistics().getHisto1D("ChannelsPerEvt").Fill( ntdc );

    auto tdcHits = evt->GetTDCChannelsArray();
    for (int i = 0; i < ntdc; ++i) {
      //const is commented because this class has inproper architecture:
      // all get-methods aren't tagged with const modifier
      auto tdcChannel = dynamic_cast </*const*/ TDCChannel * const > (tdcHits->At(i));
      auto tomb_number =  tdcChannel->GetChannel();
      if (tomb_number % 65 == 0) { // skip trigger signals from TRB
        continue;
      }
      if ( getParamBank().getTOMBChannels().count(tomb_number) == 0 ) {
        WARNING(Form("DAQ Channel %d appears in data but does not exist in the setup from DB.", tomb_number));
        continue;
      }
      JPetTOMBChannel& tomb_channel = getParamBank().getTOMBChannel(tomb_number);
      // one TDC channel may record multiple signals in one TSlot
      // iterate over all signals from one TDC channel
      // analyze number of hits per channel
      //getStatistics().getHisto1D("HitsPerEvtCh").Fill( tdcChannel->GetHitsNum() );
      const int kNumHits = tdcChannel->GetHitsNum();
      for (int j = 0; j < kNumHits; ++j) {

        // check for unreasonable times
        // the times should be negative (measured w.r.t end of time window)
        // and not smaller than -1*timeWindowWidth (which can vary for different)
        // data but shoudl not exceed 1 ms, i.e. 1.e6 ns)
        if ( tdcChannel->GetLeadTime(j) > fMaxTime ||
             tdcChannel->GetLeadTime(j) < fMinTime )continue;
        if ( tdcChannel->GetTrailTime(j) > fMaxTime ||
             tdcChannel->GetTrailTime(j) < fMinTime )continue;

        JPetSigCh sigChTmpLead = generateSigCh(tomb_channel, JPetSigCh::Leading);
        JPetSigCh sigChTmpTrail = generateSigCh(tomb_channel, JPetSigCh::Trailing);
	
		if( tomb_channel.getLocalChannelNumber() == 1 )
		{
		  getStatistics().getHisto1D("ThresholdMultiplicity_1_lead").Fill( sigChTmpLead.getPM().getID() );
		}
		else if( tomb_channel.getLocalChannelNumber() == 2 )
		{
		  getStatistics().getHisto1D("ThresholdMultiplicity_2_lead").Fill( sigChTmpLead.getPM().getID() );
		}
		else if( tomb_channel.getLocalChannelNumber() == 3 )
		{
		  getStatistics().getHisto1D("ThresholdMultiplicity_3_lead").Fill( sigChTmpLead.getPM().getID() );
		}
		else if( tomb_channel.getLocalChannelNumber() == 4 )
		{
		  getStatistics().getHisto1D("ThresholdMultiplicity_4_lead").Fill( sigChTmpLead.getPM().getID() );
		}

        // finally, set the times in ps [raw times are in ns]
        sigChTmpLead.setValue(tdcChannel->GetLeadTime(j) * 1000.);
        sigChTmpTrail.setValue(tdcChannel->GetTrailTime(j) * 1000.);
	fOutputEvents->add<JPetSigCh>(sigChTmpLead);
	fOutputEvents->add<JPetSigCh>(sigChTmpTrail);
      }
    }

    fCurrEventNumber++;
  }
}

void TimeWindowCreator::terminate() {}


void TimeWindowCreator::setParamManager(JPetParamManager* paramManager)
{
  fParamManager = paramManager;
}

const JPetParamBank& TimeWindowCreator::getParamBank() const
{
  assert(fParamManager);
  return fParamManager->getParamBank();
}

JPetSigCh TimeWindowCreator::generateSigCh(const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge) const
{
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

