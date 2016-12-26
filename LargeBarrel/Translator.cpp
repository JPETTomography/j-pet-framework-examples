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
 *  @file Translator.cpp
 */

#include <Unpacker2/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "Translator.h"

Translator::Translator(const char * name, const char * description)
:JPetTask(name, description),fCurrEventNumber(0){}

void Translator::init(const JPetTaskInterface::Options& opts){

    getStatistics().createHistogram( new TH1F("HitsPerEvtCh","Hits per channel in one event",50,-0.5,49.5) );
    getStatistics().createHistogram( new TH1F("ChannelsPerEvt","Channels fired in one event",200,-0.5,199.5) );

}

Translator::~Translator(){}

void Translator::exec(){
    if(auto evt = reinterpret_cast</*const*/ EventIII*const>(getEvent())){

        int ntdc = evt->GetTotalNTDCChannels();
        getStatistics().getHisto1D("ChannelsPerEvt").Fill( ntdc );

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

            // one TDC channel may record multiple signals in one TSlot
            // iterate over all signals from one TDC channel
            // analyze number of hits per channel
            getStatistics().getHisto1D("HitsPerEvtCh").Fill( tdcChannel->GetHitsNum() );

            const int kNumHits = tdcChannel->GetHitsNum();

            for(int j = 0; j < kNumHits; ++j){
              // check for unreasable times
              // the times should be negative (measured w.r.t end of time window)
              // and not smaller than -1*timeWindowWidth (which can vary for different)
              // data but shoudl not exceed 1 ms, i.e. 1.e6 ns)
                          if( tdcChannel->GetLeadTime(j) > kMaxTime ||
                              tdcChannel->GetLeadTime(j) < kMinTime )continue;
                          if( tdcChannel->GetTrailTime(j) > kMaxTime ||
                              tdcChannel->GetTrailTime(j) < kMinTime )continue;

              JPetSigCh sigChTmpLead = generateSigCh(tomb_channel, JPetSigCh::Leading);
              JPetSigCh sigChTmpTrail = generateSigCh(tomb_channel, JPetSigCh::Trailing);

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

void Translator::terminate(){}

void Translator::saveTimeWindow( JPetTimeWindow slot){
    assert(fWriter);
    fWriter->write(slot);
}

void Translator::setWriter(JPetWriter* writer){
    fWriter=writer;
}

void Translator::setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
}

const JPetParamBank& Translator::getParamBank()const{
    assert(fParamManager);
    return fParamManager->getParamBank();
}

JPetSigCh Translator::generateSigCh(const JPetTOMBChannel & channel, JPetSigCh::EdgeType edge) const{

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
