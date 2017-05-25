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

//ClassImp(TaskE);

TaskE::TaskE(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskE::init(const JPetTaskInterface::Options&)
{
  fOutputEvents = new JPetTimeWindow("JPetLOR");
  
  // initialize some scalar counters
  getStatistics().createCounter("No. initial hits");
  getStatistics().createCounter("No. found LORs");
}


void TaskE::exec()
{
  // A dummy analysis example:
  auto & timeWindow = *(dynamic_cast<const JPetTimeWindow* const>(getEvent()));
  getStatistics().getCounter("No. initial hits") += timeWindow.getNumberOfEvents();

  createLORs(timeWindow);

}

void TaskE::terminate()
{
  INFO( Form("From %d initial hits %d LORs were paired.", 
	     static_cast<int>(getStatistics().getCounter("No. initial hits")),
	     static_cast<int>(getStatistics().getCounter("No. found LORs")) )
	);
}

void TaskE::createLORs(const JPetTimeWindow & hits)
{
  int nhits = hits.getNumberOfEvents();
    
  for(int i = 0; i < nhits; ++i){
    for(int j=i; j< nhits; ++j){

      const JPetHit & hit1 = dynamic_cast<const JPetHit&>(hits[i]);
      const JPetHit & hit2 = dynamic_cast<const JPetHit&>(hits[j]);

      if (hit1.getScintillator() != hit2.getScintillator()) {
        // found 2 hits in different scintillators -> an event!

        // create an event object
        JPetLOR event;
        // convention: "first hit" is the one with earlier time
        if (hit1.getTime() < hit2.getTime()) {
          event.setFirstHit(hit1);
          event.setSecondHit(hit2);
        } else {

          event.setFirstHit(hit2);
          event.setSecondHit(hit1);
        }
        double dt = event.getFirstHit().getTime()
                    - event.getSecondHit().getTime();
        event.setTimeDiff(dt);
	fOutputEvents->add<JPetLOR>(event);
	getStatistics().getCounter("No. found LORs")++;
      }
    }
  }
}
