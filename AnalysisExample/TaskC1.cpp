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
 *  @file TaskC1.cpp
 */

#include "./TaskC1.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskC1);

TaskC1::TaskC1(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskC1::init(const JPetTaskInterface::Options& opts)
{
  getStatistics().createHistogram( new TH1F("No. leading points", "Leading edge points per signal", 
					    5, -0.5, 4.5) );
  getStatistics().createHistogram( new TH1F("No. trailing points", "Trailing edge points per signal", 
					    5, -0.5, 4.5) );
  getStatistics().createHistogram( new TH2F("leading vs trailing", "Leading vs trailing points per signal",
					    5, -0.5, 4.5, 5, -0.5, 4.5) );
  getStatistics().createHistogram( new TH1F("No. of fired PM-s", "Fired PMs per event", 9, -0.5, 8.5) );
  getStatistics().createHistogram( new TH1F("PM multiplicity", "PM signal multiplicity", 60, 0.5, 60.5) );

}

void TaskC1::exec()
{
  // A dummy analysis example:
  auto tslot = (JPetTimeWindow&) (*getEvent());
  std::vector<JPetRawSignal> signals(getParamBank().getPMsSize());

  // get number of SigCh's in a tslot
  const auto nSigChs = tslot.getNumberOfSigCh();
  // iterate over SigCh's in the Tslot and join them in signals
  const auto kPMsSize = getParamBank().getPMsSize();
  for (auto i = 0; i < nSigChs; i++) {
    JPetSigCh sigch = tslot[i];

    int index = 0;
    while (index < kPMsSize
        && getParamBank().getPM(index) != sigch.getPM())
    {
        index++;
    }
    signals.at(index).addPoint(sigch);
    //signalsArray[index].addPoint(sigch);
  }

  int nPMs = 0;
  
  for (int i = 0; i < kPMsSize; i++) {
    JPetRawSignal sig = signals.at(i);
    //JPetRawSignal sig = signalsArray[i];

    if (sig.getNumberOfPoints(JPetSigCh::Leading) == 0
        && sig.getNumberOfPoints(JPetSigCh::Trailing) == 0) { //skip empty signals
      continue;
    }
    nPMs++; // count how many PM-s fired in one TimeWindow

    sig.setTimeWindowIndex(tslot.getIndex());
    sig.setPM(getParamBank().getPM(i));

    // keep some statistics
    getStatistics().getHisto1D("No. leading points").Fill(sig.getNumberOfPoints(JPetSigCh::Leading));
    getStatistics().getHisto1D("No. trailing points").Fill(
        sig.getNumberOfPoints(JPetSigCh::Trailing));
    getStatistics().getHisto2D("leading vs trailing").Fill(
        sig.getNumberOfPoints(JPetSigCh::Leading),
        sig.getNumberOfPoints(JPetSigCh::Trailing));

    getStatistics().getHisto1D("PM multiplicity").Fill(sig.getPM().getID());

    // consider events with at least 3 fired thresholds
    if (sig.getNumberOfPoints(JPetSigCh::Leading) >= 2
        && sig.getNumberOfPoints(JPetSigCh::Trailing) >= 2) {

      // if the signal is worth keeping, write the signal to output tree
      saveRawSignal(sig);
    }

  }

  getStatistics().getHisto1D("No. of fired PM-s").Fill(nPMs);
}

void TaskC1::terminate()
{
}


void TaskC1::saveRawSignal( JPetRawSignal sig)
{
  assert(fWriter);
  fWriter->write(sig);
}
