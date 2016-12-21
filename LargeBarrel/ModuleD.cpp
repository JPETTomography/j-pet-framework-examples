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
 *  @file ModuleD.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include "ModuleD.h"

using namespace std;

ModuleD::ModuleD(const char * name, const char * description):JPetTask(name, description){}

ModuleD::~ModuleD(){}

void ModuleD::init(const JPetTaskInterface::Options& opts){
}

void ModuleD::exec(){
    //getting the data from event in propriate format
    if(auto currSignal = dynamic_cast<const JPetPhysSignal*const>(getEvent())){

        if (fSignals.empty()) {
            fSignals.push_back(*currSignal);
        }

        else {
            if (fSignals[0].getRecoSignal().getRawSignal().getTimeWindowIndex() == currSignal->getRecoSignal().getRawSignal().getTimeWindowIndex()) {
                fSignals.push_back(*currSignal);
            }
            else {
                saveHits(createHits(fSignals));
                fSignals.clear();
                fSignals.push_back(*currSignal);
            }
        }
    }
}

vector<JPetHit> ModuleD::createHits(const vector<JPetPhysSignal>&signals){

    vector<JPetHit> hits;

    for (auto i = signals.begin(); i != signals.end(); ++i) {
        for (auto j = i; ++j != signals.end();) {

            if (i -> getRecoSignal().getRawSignal().getPM().getScin() == j -> getRecoSignal().getRawSignal().getPM().getScin()) {
                // found 2 signals from the same scintillator

                // assign sides A and B properly

                if(
                    (i->getRecoSignal().getRawSignal().getPM().getSide() == JPetPM::SideA)
                    &&(j->getRecoSignal().getRawSignal().getPM().getSide() == JPetPM::SideB)
                ){
                    if( abs(i -> getTime() - j -> getTime()) < TIME_WINDOW /*ps*/){

                        JPetPhysSignal signalA = *i;
                        JPetPhysSignal signalB = *j;
                        JPetHit hit;
                        hit.setSignalA(signalA);
                        hit.setSignalB(signalB);
                        hit.setTime( (signalA -> getTime() + signalB -> getTime())/2.0 );
                        hit.setScintillator(i -> getRecoSignal().getRawSignal().getPM().getScin());
                        hit.setBarrelSlot(i -> getRecoSignal().getRawSignal().getPM().getScin().getBarrelSlot());

                        hits.push_back(hit);
                    }
                }
                else {
                    // if two hits on the same side, ignore
                    WARNING("TWO hits on the same scintillator side we ignore it");
                    continue;
                }


            }
        }
    }
    return hits;
}

void ModuleD::terminate(){
    saveHits(createHits(fSignals)); //if there is something left
}


void ModuleD::saveHits(const vector<JPetHit>& hits){
    assert(fWriter);
    JPetAnalysisTools sort;
    auto sortedHits = sort.getHitsOrderedByTime(hits);

    for (auto hit : sortedHits){
        // here one can impose any conditions on hits that should be
        // saved or skipped
        // for now, all hits are written to the output file
        // without checking anything
        fWriter->write(hit);
    }
}
void ModuleD::setWriter(JPetWriter* writer){fWriter =writer;}
