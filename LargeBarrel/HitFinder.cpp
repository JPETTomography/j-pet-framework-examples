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
 *  @file HitFinder.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include "HitFinder.h"

using namespace std;

HitFinder::HitFinder(const char * name, const char * description):JPetTask(name, description){}

HitFinder::~HitFinder(){}

void HitFinder::init(const JPetTaskInterface::Options& opts){
}

void HitFinder::exec(){
    //getting the data from event in propriate format
    if(auto currSignal = dynamic_cast<const JPetPhysSignal*const>(getEvent())){


        if (DAQTimeWindowIndex == -1) {
            DAQTimeWindowIndex = currSignal->getRecoSignal().getRawSignal().getTimeWindowIndex();
            fillSignalsMap(*currSignal);

        }

        else {
            if (DAQTimeWindowIndex == currSignal->getRecoSignal().getRawSignal().getTimeWindowIndex()) {
                fillSignalsMap(*currSignal);
            }
            else {
                saveHits(createHits( fAllSignalsInTimeWindow, kTimeWindowWidth));
                fAllSignalsInTimeWindow.clear();
                fillSignalsMap(*currSignal);
            }
        }
    }
}


vector<JPetHit> HitFinder::createHits(const SignalsContainer& allSignalsInTimeWindow, const double timeDifferenceWindow){


    // This method takes signal from side A on a scintilator and compares it with signals on side B - if they are within time window then it creates hit

    vector<JPetHit> hits;

    for (auto scintillator : allSignalsInTimeWindow) {

        auto sideA = scintillator.second.first;
        auto sideB = scintillator.second.second;

        if(sideA.size() > 0 and sideB.size() > 0){

            for(auto signalA : sideA){
                for(auto signalB : sideB){

                    if(abs(signalA.getTime() - signalB.getTime()) < timeDifferenceWindow /*ps*/){

                        JPetHit hit;
                        hit.setSignalA(signalA);
                        hit.setSignalB(signalB);
                        hit.setTime( (signalA.getTime() + signalB.getTime())/2.0 );
                        hit.setScintillator(signalA.getRecoSignal().getRawSignal().getPM().getScin());
                        hit.setBarrelSlot(signalA.getRecoSignal().getRawSignal().getPM().getScin().getBarrelSlot());

                        hits.push_back(hit);
                    }
                }
            }

        }
    }
    return hits;
}

void HitFinder::terminate(){
    saveHits(createHits(fAllSignalsInTimeWindow, kTimeWindowWidth)); //if there is something left
}


void HitFinder::saveHits(const vector<JPetHit>& hits){
    assert(fWriter);
    auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);

    for (const auto & hit : sortedHits){
        // here one can impose any conditions on hits that should be
        // saved or skipped
        // for now, all hits are written to the output file
        // without checking anything
        fWriter->write(hit);
    }
}

void HitFinder::setWriter(JPetWriter* writer){fWriter =writer;}

void HitFinder::fillSignalsMap(JPetPhysSignal signal){

    if(signal.getRecoSignal().getRawSignal().getPM().getSide() == JPetPM::SideA){

        fAllSignalsInTimeWindow.at(signal.getRecoSignal().getRawSignal().getPM().getScin().getID()).first.push_back(signal);
    }
    else{

        fAllSignalsInTimeWindow.at(signal.getRecoSignal().getRawSignal().getPM().getScin().getID()).second.push_back(signal);
    }

};
