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
#include <JPetGeomMapping/JPetGeomMapping.h>

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

  // take coordinates of the main (irradiated strip) from user parameters
  if (opts.count(kMainStripKey)) {
    fMainStripSet = true;
    int code = std::atoi(opts.at(kMainStripKey).c_str());
    fMainStrip.first = code / 100;  // layer number
    fMainStrip.second = code % 100; // strip number
    
    // build a list of allowed channels
    JPetGeomMapping mapper(fParamManager->getParamBank());
    
    for(int thr=1;thr<=4;++thr){
      int tomb_number = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideA, thr);
      fAllowedChannels.insert(tomb_number);
      tomb_number = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideB, thr);
      fAllowedChannels.insert(tomb_number);
    }
    
    // add all reference detector channels to allowed channels list
    for(int thr=1;thr<=4;++thr){
      int tomb_number = mapper.getTOMB(4, 1, JPetPM::SideA, thr);
      fAllowedChannels.insert(tomb_number);
      tomb_number = mapper.getTOMB(4, 1, JPetPM::SideB, thr);
      fAllowedChannels.insert(tomb_number);
    }
    
  }
  
  getStatistics().createHistogram( new TH1F("HitsPerEvtCh", "Hits per channel in one event", 50, -0.5, 49.5) );
  getStatistics().createHistogram( new TH1F("ChannelsPerEvt", "Channels fired in one event", 200, -0.5, 199.5) );
}

TimeWindowCreator::~TimeWindowCreator() {}

void TimeWindowCreator::exec()
{
  //getting the data from event in apropriate format
  //const is commented because this class has inproper architecture:
  // all get-methods aren't tagged with const modifier
  if (auto evt = dynamic_cast </*const*/ EventIII * const > (getEvent())) {
    int ntdc = evt->GetTotalNTDCChannels();
    getStatistics().getHisto1D("ChannelsPerEvt").Fill( ntdc );

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

      // ignore irrelevant channels
      // (for time calibration)
      if ( !filter(tomb_channel) ){
	continue;
      }
      

      // one TDC channel may record multiple signals in one TSlot
      // iterate over all signals from one TDC channel
      // analyze number of hits per channel
      getStatistics().getHisto1D("HitsPerEvtCh").Fill( tdcChannel->GetHitsNum() );
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

/**
 * Returns true if signal from the channel given as argument should be passed
 */
bool TimeWindowCreator::filter(const JPetTOMBChannel& channel) const{

  if( !fMainStripSet ){ // if main strip was not defined, pass all channels
    return true;
  }

  if( fAllowedChannels.find(channel.getChannel()) != fAllowedChannels.end() ){
    return true;
  }
  
  return false;
}

