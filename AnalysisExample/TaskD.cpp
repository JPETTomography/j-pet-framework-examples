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
 *  @file TaskD.cpp
 */

#include "./TaskD.h"
#include "JPetWriter/JPetWriter.h"
#include <iostream>

//ClassImp(TaskD);

TaskD::TaskD(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskD::init(const JPetTaskInterface::Options& opts)
{
  getStatistics().createHistogram( new TH1F("No. signals in TSlot", "Signals multiplicity per TSlot", 10,
					    -0.5, 9.5) );
  getStatistics().createHistogram( new TH1F("Scins multiplicity", "scintillators multiplicity", 65, 5.5,
					    70.5));

  // create a histogram of time difference for every scintillator in the setup
  // we loop over all scintillator objects in the parameters bank downloaded
  // from database for the run number given by user
  for(auto & scin : getParamBank().getScintillators()){
    TString name = Form("dt for scin %d", scin.first);
    getStatistics().createHistogram( new TH1F(name, name, 100, -10000., 10000.) );
  }
  
  // initialize some scalar counters
  getStatistics().createCounter("No. initial signals");
  getStatistics().createCounter("No. found hits");
}


void TaskD::exec()
{
  // A dummy analysis example:
  JPetPhysSignal currSignal = (JPetPhysSignal&) (*getEvent());
  
  // increment the counter of signals
  getStatistics().getCounter("No. initial signals")++;

  if (fSignals.empty()) {
    fSignals.push_back(currSignal);
  } else {
    if (fSignals[0].getTimeWindowIndex() == currSignal.getTimeWindowIndex()) {
      fSignals.push_back(currSignal);
    } else {
      getStatistics().getHisto1D("No. signals in TSlot").Fill(fSignals.size());
      saveHits(createHits(fSignals)); //create LORs from previously saved signals
      fSignals.clear();
      fSignals.push_back(currSignal);
    }
  }
}

std::vector<JPetHit> TaskD::createHits(std::vector<JPetPhysSignal>& signals)
{
  std::vector<JPetHit> hits;
  for (auto i = signals.begin(); i != signals.end(); ++i) {
    for (auto j = i; ++j != signals.end(); /**/) {
      if (i->getPM().getScin() == j->getPM().getScin()) {
        // found 2 signals from the same scintillator
        JPetHit hit;
        if (i->getPM().getSide() == JPetPM::SideA
            && j->getPM().getSide() == JPetPM::SideB) {
          hit.setSignalA(*i);
          hit.setSignalB(*j);
        } else if (j->getPM().getSide() == JPetPM::SideA
                   && i->getPM().getSide() == JPetPM::SideB) {
          hit.setSignalA(*j);
          hit.setSignalB(*i);
        } else {
          WARNING("TWO hits on the same scintillator side we ignore it");
          // if two hits on the same side, ignore
          continue;
        }

        hit.setScintillator(i->getPM().getScin());

        getStatistics().getHisto1D("Scins multiplicity").Fill((*i).getPM().getScin().getID());

        double dt = hit.getSignalA().getTime() - hit.getSignalB().getTime();
        hit.setTimeDiff(dt);

        double t = 0.5
                   * (hit.getSignalA().getTime() + hit.getSignalB().getTime());
        hit.setTime(t);

	// fill the appropriate dt histogram for this scintillator                                                   
	getStatistics().getHisto1D( 
				   Form("dt for scin %d", hit.getScintillator().getID())
				    ).Fill(hit.getTimeDiff());


        hits.push_back(hit);
	// increment the counter of found hits
	getStatistics().getCounter("No. found hits")++;
      }
    }
  }
  return hits;
}

void TaskD::terminate()
{
  std::cout <<"terminate() Taskd()" <<std::endl;
  std::cout <<"number of signals:"<<fSignals.size() <<std::endl;
  for(auto sig: fSignals) {
    std::cout<<"some signal info:"<<sig.getPM().getScin().getID()<<std::endl;
    std::cout<<"unique id:"<<sig.GetUniqueID()<<std::endl;
  }
  //  saveHits(createHits(fSignals)); //if there is something left
  INFO( Form("From %d initial signals %d hits were paired.", 
	     static_cast<int>(getStatistics().getCounter("No. initial signals")),
	     static_cast<int>(getStatistics().getCounter("No. found hits")) )
	);
}


void TaskD::saveHits(std::vector<JPetHit> hits)
{
  assert(fWriter);
  for (auto hit : hits) { 
    fWriter->write(hit);
  }
}
