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
 *  @file PreEventFinder.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetEvent/JPetEvent.h>
#include "PreEventFinder.h"
// #include "PreEventFinderTools.h"

using namespace std;

PreEventFinder::PreEventFinder(const char* name, const char* description): JPetTask(name, description) {}

PreEventFinder::~PreEventFinder() {}

void PreEventFinder::init(const JPetTaskInterface::Options& opts)
{

}

void PreEventFinder::exec()
{
  //getting the data from event in propriate format
  if (auto currHit = dynamic_cast<const JPetHit* const>(getEvent())) {

    if (fHits.empty()) {
            fHits.push_back(*currHit);
        }
        else {
            if (fHits[0].getSignalB().getTimeWindowIndex() == currHit->getSignalB().getTimeWindowIndex()) {
                fHits.push_back(*currHit);
            } else {
                SavePreEvents(CreatePreEvents(fHits, kSubsequentHitsTimeDiff));
                fHits.clear();
                fHits.push_back(*currHit);
            }
        }


  }
}




void PreEventFinder::terminate()
{
    SavePreEvents(CreatePreEvents(fHits, kSubsequentHitsTimeDiff));
}


void PreEventFinder::setWriter(JPetWriter* writer)
{
  fWriter = writer;
}


vector<JPetEvent> PreEventFinder::CreatePreEvents( vector<JPetHit> &Hits, double subsequentHitsTimeDiff){

  auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(Hits);

  vector<JPetEvent> PreEvents;
  JPetEvent PreEvent;

  int hitsInTimeWindow = sortedHits.size();
  int preEventNumber = 0;

  PreEvents.insert(PreEvents.end(), PreEvent);
  PreEvents.at(0).addHit(sortedHits[0]);

  if(hitsInTimeWindow > 1){
    for(int i = 1; i < hitsInTimeWindow; i++){

      if( (sortedHits[i].getTime() - sortedHits[i-1].getTime()) > subsequentHitsTimeDiff){
        preEventNumber++;
        PreEvents.insert(PreEvents.end(), PreEvent);
      }
      PreEvents[preEventNumber].addHit(sortedHits[i]);
    }
  }


  return PreEvents;
}

void PreEventFinder::SavePreEvents( vector<JPetEvent> PreEventsInTimeWindow){

 assert(fWriter);

  for (const auto & preEvent : PreEventsInTimeWindow) {
    // here one can impose any conditions on hits that should be
    // saved or skipped
    // for now, all hits are written to the output file
    // without checking anything
    fWriter->write(preEvent);
  }

}
