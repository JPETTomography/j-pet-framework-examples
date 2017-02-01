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
 *  @file TaskC2.cpp
 */

#include "./TaskC2.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskC2);

TaskC2::TaskC2(const char* name, const char* description):
  JPetTask(name, description)
{
}

void TaskC2::init(const JPetTaskInterface::Options& opts)
{
  getStatistics().createHistogram( new TH1F("tot", "Time over threshold", 100, 0., 5000.) );
}


void TaskC2::exec()
{
  // A dummy analysis example:
  auto currSignal = (JPetRawSignal&) (*getEvent());
  auto recoSignal = createRecoSignal(currSignal);
  savePhysSignal(createPhysSignal(recoSignal));
}

void TaskC2::terminate()
{
}

JPetRecoSignal TaskC2::createRecoSignal(JPetRawSignal& rawSignal)
{
  JPetRecoSignal recoSignal;

  // example dummy analysis - calculate Time Over Threshold:

  // first, get vectors of SigCh-s from both edges, sorted by their threshold values
  std::vector<JPetSigCh> leadingPoints = rawSignal.getPoints(
      JPetSigCh::Leading, JPetRawSignal::ByThrValue);
  std::vector<JPetSigCh> trailingPoints = rawSignal.getPoints(
      JPetSigCh::Trailing, JPetRawSignal::ByThrValue);
  // if the values of thresholds were not set i the database, you may want to use
  // JPetRawSignal::ByThrNum option instead which will sort the SigCh-s by the number
  // of threshold on its edge (1,..,4 in this case) rather by threshold value

  // on each edge, take the first (smallest) threshold and its corresponding time
  double t1 = leadingPoints.front().getValue();
  double t2 = trailingPoints.front().getValue();
  // note that there is no guarantee that these two smallest thresholds will actually
  // have the same threshold value: for some reason, time on the smallest threshold
  // might not have been recorded on one of the edges
  // this should be checked, but it is beyond the scope of this simple example

  // finally, calculate the ToT
  double ToT = t2 - t1;

  // now that we have the ToT, we can calculate charge from it
  // here, a dummy calculation
  double charge = ToT * 1.0 + 0.0;

  // and write the tot value to a histogram
  getStatistics().getHisto1D("tot").Fill(ToT);

  // store the newly calculated charge value in the RecoSignal
  recoSignal.setCharge( charge );

  // similarly to the above example, we can calculate other properties
  // and store them
  recoSignal.setDelay( 0.0 );
  recoSignal.setOffset( 0.0 );
  recoSignal.setAmplitude( 0.0 );

  // one more silly example - calculate time at arbitrary threshold by "interpolation"
  // between lowest and second-lowest threshold measured by DAQ on leading edge
  double thr1, thr2;
  t1 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getValue();
  thr1 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getThreshold();
  t2 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(1).getValue();
  thr2 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(1).getThreshold();

  recoSignal.setRecoTimeAtThreshold((thr1 + thr2) / 2., (t1 + t2) / 2. );
  //recoSignal.setRecoTimeAtThreshold(10., 20. );

  // store the original JPetRawSignal in the RecoSignal as a processing history
  recoSignal.setRawSignal(rawSignal);
  return recoSignal;
}

JPetPhysSignal TaskC2::createPhysSignal(JPetRecoSignal& recoSignal)
{
  JPetPhysSignal physSignal;

  // use the values from Reco Signal to set the physical properties of signal
  // here, a dummy example - much more sophisticated procedures should go here
  physSignal.setPhe( recoSignal.getCharge() * 1.0 + 0.0 );
  physSignal.setQualityOfPhe(1.0);

  // we retrieve it by getting a map of times vs. thresholds,
  // and taking its first (and only) element by the begin() iterator. We get
  // an std::pair, where first is the threshold value, and second is time.
  double time = recoSignal.getRecoTimesAtThreshold().begin()->second;
  physSignal.setTime(time);
  physSignal.setQualityOfTime(1.0);

  // store the original JPetRecoSignal in the PhysSignal as a processing history
  physSignal.setRecoSignal(recoSignal);
  return physSignal;
}

void TaskC2::savePhysSignal( JPetPhysSignal sig)
{
  assert(fWriter);
  fWriter->write(sig);
}

