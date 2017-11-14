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
 *  @file TaskB.cpp
 */

#include "./TaskB.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskB);

TaskB::TaskB(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskB::init(const JPetTaskInterface::Options&)
{
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  getStatistics().createHistogram( new TH1F("single threshold multiplicity",
					    "Single threshold multipicity within single time window",
					    10, -0.5, 9.5)
				   );
}


void TaskB::exec()
{
  auto & timeWindow = *(dynamic_cast<const JPetTimeWindow* const>(getEvent()));

  // get number of SigCh-s in a tslot
  auto nSigChs = timeWindow.getNumberOfEvents();

  // we would like to check if a signal from one DAQ channel can occur more than once
  // during one Time Slot, so we will create a map(channel, No. of occurences) to count multiplicities
  std::map<int,int> channelMultiplicities;

  // iterate over SigCh's in the time window and calibrate their times
  for (int i = 0; i < nSigChs; i++) {
    const JPetSigCh & sigchRaw = dynamic_cast<const JPetSigCh&>(timeWindow[i]);
    JPetSigCh sigchCal = sigchRaw; // start the calibrated SigCh as a copy of the raw one

    // do our multiplicity counting
    channelMultiplicities[sigchRaw.getDAQch()]++;

    // a dummy "calibration" example; real calibration should go here
    float time = sigchRaw.getValue();
    time = time * 1.0 + 0.0;
    // set time after calibration in the new SigCh object
    sigchCal.setValue(time);

    // insert the calibrated SigCh into calibrated TimeWindow
    fOutputEvents->add<JPetSigCh>(sigchCal);
  }

  // write all non-zero multiplicities to a histogram
  for (std::map<int,int>::iterator it=channelMultiplicities.begin(); it!=channelMultiplicities.end(); ++it){
    if( it->second > 0 ){
      getStatistics().getHisto1D("single threshold multiplicity").Fill(it->second);
    }
  }
  
}

void TaskB::terminate()
{
}
