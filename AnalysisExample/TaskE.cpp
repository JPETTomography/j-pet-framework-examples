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
 *  @file TaskE.cpp
 */

#include "./TaskE.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskE);

TaskE::TaskE(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskE::init(const JPetTaskInterface::Options& opts)
{
  // initialize some scalar counters
  getStatistics().createCounter("No. initial hits");
  getStatistics().createCounter("No. found LORs");
}


void TaskE::exec()
{
  // A dummy analysis example:
  JPetHit currHit = (JPetHit&) (*getEvent());
  getStatistics().getCounter("No. initial hits")++;

  if (fHits.empty()) {
    fHits.push_back(currHit);
  } else {
    if (fHits[0].getTimeWindowIndex() == currHit.getSignalB().getTimeWindowIndex()) {
      fHits.push_back(currHit);
    } else {
      saveLORs(createLORs(fHits)); //create LORs from previously saved signals
      fHits.clear();
      fHits.push_back(currHit);
    }
  }
}

void TaskE::terminate()
{
  saveLORs(createLORs(fHits)); //if there is something left

  INFO( Form("From %d initial hits %d LORs were paired.", 
	     static_cast<int>(getStatistics().getCounter("No. initial hits")),
	     static_cast<int>(getStatistics().getCounter("No. found LORs")) )
	);
}

std::vector<JPetLOR> TaskE::createLORs(std::vector<JPetHit>& hits)
{
  std::vector<JPetLOR> lors;
  for (auto i = hits.begin(); i != hits.end(); ++i) {
    for (auto j = i; ++j != hits.end(); /**/) {
      if (i->getScintillator() != j->getScintillator()) {
        // found 2 hits in different scintillators -> an event!

        // create an event object
        JPetLOR event;
        // convention: "first hit" is the one with earlier time
        if (i->getTime() < j->getTime()) {
          event.setFirstHit(*i);
          event.setSecondHit(*j);
        } else {

          event.setFirstHit(*j);
          event.setSecondHit(*i);
        }
        double dt = event.getFirstHit().getTime()
                    - event.getSecondHit().getTime();
        event.setTimeDiff(dt);
        lors.push_back(event);
	getStatistics().getCounter("No. found LORs")++;
      }
    }
  }
  return lors;
}

void TaskE::saveLORs(std::vector<JPetLOR> lors)
{
  for (auto lor : lors) {
    fWriter->write(lor);
  }
}
