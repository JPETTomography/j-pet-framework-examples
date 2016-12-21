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
 *  @file ModuleB.cpp
 */

#include <map>
#include <vector>
#include <string>
#include <JPetWriter/JPetWriter.h>
#include "ModuleB.h"

using namespace std;

ModuleB::ModuleB(const char * name, const char * description):
JPetTask(name, description){}
ModuleB::~ModuleB(){}

void ModuleB::init(const JPetTaskInterface::Options& opts){

    getStatistics().createHistogram( new TH1F("Lead", "Number of points on leading edge", 18, -0.5, 17.5) );
    // getStatistics().createHistogram( new TH1F("Trail", "Points on trailing edge", 18, -0.5, 17.5) );
    for (int thr=1; thr<=4; thr++){

        const char * histo_title = Form("Number of points on thr %d", thr);
        const char * histo_name = Form("PointsOnThr%d", thr);
        getStatistics().createHistogram( new TH1F(histo_name, histo_name, 6, -0.5, 5.5) );
    }

    getStatistics().createHistogram( new TH1F("thr1=0","Total number of points if there was no info at thr1" , 16, -0.5, 15.5) );
    getStatistics().createHistogram( new TH1F("tdiff","Time diff" , 2*pow(10,6), -pow(10,9), pow(10,9)));

}

void ModuleB::exec(){
    //getting the data from event in propriate format
    if(auto timeWindow = dynamic_cast<const JPetTimeWindow*const>(getEvent())){

        map<int,vector<JPetSigCh>> leadSigChs;
        map<int,vector<JPetSigCh>> trailSigChs;
        map<int,JPetRawSignal> signals;

        const size_t nSigChs = timeWindow->getNumberOfSigCh();

        for (auto i = 0; i < nSigChs; i++) {

            JPetSigCh sigch = timeWindow->operator[](i);
            int daq_channel = sigch.getChannel();

            if( sigch.getType() == JPetSigCh::Leading )
                leadSigChs[ daq_channel ].push_back(sigch);
            if( sigch.getType() == JPetSigCh::Trailing )
                trailSigChs[ daq_channel ].push_back(sigch);
        }

        // // iterate over the leading-edge SigChs
        for (auto & chSigPair : leadSigChs) {

            int daq_channel = chSigPair.first;

            int signalsPerChannel = chSigPair.second.size();

            if( trailSigChs.count(daq_channel) != 0 ){

                for(int daq_signal = 0; daq_signal < signalsPerChannel; daq_signal++){

                JPetSigCh & leadSigCh = chSigPair.second.at(daq_signal);
                JPetSigCh & trailSigCh = trailSigChs.at(daq_channel).at(daq_signal);

        //         // double tot = trailSigCh.getValue() - leadSigCh.getValue();

                if( leadSigCh.getPM() != trailSigCh.getPM() ){
                    ERROR("Signals from same channel point to different PMTs! Check the setup mapping!!!");
                }

                double pmt_id = trailSigCh.getPM().getID();
                signals[pmt_id].addPoint( leadSigCh );
                signals[pmt_id].addPoint( trailSigCh );

                }
            }
        }


        for(auto & pmSignalPair : signals){

            auto & pmSignals = pmSignalPair.second;

            auto leadSig = pmSignals.getPoints(JPetSigCh::Leading);

            vector<JPetSigCh> thr1, thr2, thr3, thr4;

            for(auto & point : leadSig){

                if(point.getThresholdNumber() == 1) thr1.push_back(point);
                if(point.getThresholdNumber() == 2) thr2.push_back(point);
                if(point.getThresholdNumber() == 3) thr3.push_back(point);
                if(point.getThresholdNumber() == 4) thr4.push_back(point);
            }

            getStatistics().getHisto1D("PointsOnThr1").Fill(thr1.size());
            getStatistics().getHisto1D("PointsOnThr2").Fill(thr2.size());
            getStatistics().getHisto1D("PointsOnThr3").Fill(thr3.size());
            getStatistics().getHisto1D("PointsOnThr4").Fill(thr4.size());

            if(thr1.size() == 0) getStatistics().getHisto1D("thr1=0").Fill(thr2.size() + thr3.size() + thr4.size());

            getStatistics().getHisto1D("Lead").Fill(leadSig.size());

            sort(thr1.begin(), thr1.end(), [](const JPetSigCh & hit1, const JPetSigCh & hit2){return hit1.getValue() < hit2.getValue();});
            if(thr1.size() > 1){

                for(int i = 1; i < thr1.size(); i++ ){

                    getStatistics().getHisto1D("tdiff").Fill(thr1[i].getValue() - thr1[i-1].getValue());
                }
            }
        }


            // signal.setTimeWindowIndex( timeWindow->getIndex() );

            // const auto & pmt = getParamBank().getPM(pmSignalPair.first);

            // signal.setPM(pmt);
            // signal.setBarrelSlot(pmt.getBarrelSlot());

            // fWriter->write(signal);


    }
}

void ModuleB::terminate(){}

void ModuleB::saveRawSignal( JPetRawSignal sig){
    assert(fWriter);
    fWriter->write(sig);
}

void ModuleB::setWriter(JPetWriter* writer) {
    fWriter = writer;
}
void ModuleB::setParamManager(JPetParamManager* paramManager) {
    fParamManager = paramManager;
}
const JPetParamBank& ModuleB::getParamBank() const {
    return fParamManager->getParamBank();
}
