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
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include "TaskC.h"
#include <algorithm>

using namespace std;


TaskC::TaskC(const char* name, const char* description): JPetTask(name, description) {}
TaskC::~TaskC() {}

void TaskC::init(const JPetTaskInterface::Options&)
{ 
  fOutputEvents = new JPetTimeWindow("JPetHit");
}

void TaskC::exec()
{
  //getting the data from event in propriate format
  if (auto timeWindow = dynamic_cast<JPetTimeWindow* const>(getEvent())) {
    getStatistics().getCounter("No. initial signals") += timeWindow->getNumberOfEvents();

    vector<JPetHit> hits = createHits(*timeWindow);
    hits = JPetAnalysisTools::getHitsOrderedByTime(hits);

    saveHits(hits);
  }
}
vector<JPetHit> TaskC::createHits(const JPetTimeWindow & signals)
{
  uint nsignals = signals.getNumberOfEvents();
  
  vector<JPetHit> hits;
  for(uint i = 0; i < nsignals; ++i){
    for(uint j=i+1; j< nsignals; ++j){
      const JPetRawSignal & rawSignalA = dynamic_cast<const JPetRawSignal&>(signals[i]);
      const JPetRawSignal & rawSignalB = dynamic_cast<const JPetRawSignal&>(signals[j]);
      if (rawSignalA.getPM().getScin() == rawSignalB.getPM().getScin()) {
        // found 2 signals from the same scintillator
        // wrap the RawSignal objects into RecoSignal and PhysSignal
        // for now this is just wrapping opne object into another
        // in the future analyses it will involve more logic like
        // reconstructing the signal's shape, charge, amplitude etc.
        JPetRecoSignal recoSignalA;
        JPetRecoSignal recoSignalB;
        JPetPhysSignal physSignalA;
        JPetPhysSignal physSignalB;
        // assign sides A and B properly
        if (
          (rawSignalA.getPM().getSide() == JPetPM::SideA)
          && (rawSignalB.getPM().getSide() == JPetPM::SideB)
        ) {
          recoSignalA.setRawSignal(rawSignalA);
          recoSignalB.setRawSignal(rawSignalB);
        } else if (
          (rawSignalB.getPM().getSide() == JPetPM::SideA)
          && (rawSignalA.getPM().getSide() == JPetPM::SideB)
        ) {
          recoSignalA.setRawSignal(rawSignalB);
          recoSignalB.setRawSignal(rawSignalA);
        } else {
          // if two hits on the same side, ignore
          WARNING("TWO hits on the same scintillator side we ignore it");
          continue;
        }

        if ( recoSignalA.getRawSignal().getNumberOfPoints(JPetSigCh::Leading) < 4 ) continue;
        if ( recoSignalB.getRawSignal().getNumberOfPoints(JPetSigCh::Leading) < 4 ) continue;

        bool thresholds_ok = true;
        for (uint i = 1; i <= 4; ++i) {
          if ( recoSignalA.getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).count(i) < 1 ) {
            thresholds_ok = false;
          }
          if ( recoSignalB.getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).count(i) < 1 ) {
            thresholds_ok = false;
          }
        }
        if (thresholds_ok == false) {
          continue;
        }

        physSignalA.setRecoSignal(recoSignalA);
        physSignalB.setRecoSignal(recoSignalB);
        auto leading_points_a = physSignalA.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto leading_points_b = physSignalB.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

        //skip signals with no information on 1st threshold
        // if(leading_points_a.count(1) == 0) continue;
        // if(leading_points_b.count(1) == 0) continue;

        physSignalA.setTime(leading_points_a.at(1));
        physSignalB.setTime(leading_points_b.at(1));


        JPetHit hit;
        hit.setSignalA(physSignalA);
        hit.setSignalB(physSignalB);
        hit.setScintillator(rawSignalA.getPM().getScin());
        hit.setBarrelSlot(rawSignalA.getPM().getScin().getBarrelSlot());

        physSignalA.setTime(physSignalA.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).at(1));
        physSignalB.setTime(physSignalB.getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading).at(1));

        hit.setTime( 0.5 * ( hit.getSignalA().getTime() + hit.getSignalB().getTime()) );

        hits.push_back(hit);
        getStatistics().getCounter("No. found hits")++;
      }
    }
  }
  return hits;
}

void TaskC::terminate()
{
  INFO( Form("From %d initial signals %d hits were paired.",
             static_cast<int>(getStatistics().getCounter("No. initial signals")),
             static_cast<int>(getStatistics().getCounter("No. found hits")) )
      );
}

void TaskC::saveHits(const vector<JPetHit>& hits)
{
  for (auto hit : hits) {
    // here one can impose any conditions on hits that should be
    // saved or skipped
    // for now, all hits are written to the output file
    // without checking anything
    fOutputEvents->add<JPetHit>(hit);
  }
}
