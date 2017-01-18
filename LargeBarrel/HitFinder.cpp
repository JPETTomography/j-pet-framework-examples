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
#include "HitFinderTools.h"

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
            HitTools.fillContainer(fAllSignalsInTimeWindow, *currSignal);

        }

        else {
            if (DAQTimeWindowIndex == currSignal->getRecoSignal().getRawSignal().getTimeWindowIndex()) {
                HitTools.fillContainer(fAllSignalsInTimeWindow, *currSignal);
            }
            else {
                saveHits(HitTools.createHits( fAllSignalsInTimeWindow, kTimeWindowWidth));
                fAllSignalsInTimeWindow.clear();
                HitTools.fillContainer(fAllSignalsInTimeWindow, *currSignal);
            }
        }
    }
}



void HitFinder::terminate(){
    saveHits(HitTools.createHits(fAllSignalsInTimeWindow, kTimeWindowWidth)); //if there is something left
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
