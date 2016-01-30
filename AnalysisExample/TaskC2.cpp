/**
  *  @copyright Copyright (c) 2014, Wojciech Krzemien
  *  @file TaskC2.cpp
  *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
  */

#include "./TaskC2.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskC2);

TaskC2::TaskC2(const char * name, const char * description):
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
  saveRecoSignal(createRecoSignal(currSignal));
}

void TaskC2::terminate()
{
}

JPetRecoSignal TaskC2::createRecoSignal(JPetRawSignal& rawSignal)
{
  // create a Reco Signal
  JPetRecoSignal recoSignal;

  // example dummy analysis - calculate Time Over Threshold:
  double ToT = rawSignal.getTOT(); // will return a reasonable value only if
  // the raw signal contains a ToT measurement from the electronics

  if (ToT == JPetSigCh::kUnset) { // we got JPetSigCh::kUnset value,
    // perhaps boards without ToT capability were used?
    // we calculate ToT by hand, then

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
    ToT = t2 - t1;
  }

  // now, that we have the ToT, we can calculate charge from it
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
  double t1,t2, thr1, thr2;
  t1 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getValue();
  thr1 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(0).getThreshold();
  t2 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(1).getValue();
  thr2 = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue).at(1).getThreshold();

  recoSignal.setRecoTimeAtThreshold((thr1+thr2)/2., (t1+t2)/2. );
  //recoSignal.setRecoTimeAtThreshold(10., 20. );

  // store the original JPetRawSignal in the RecoSignal as a processing history
  recoSignal.setRawSignal(rawSignal);
  return recoSignal;
}

void TaskC2::saveRecoSignal( JPetRecoSignal signal)
{
  assert(fWriter);
  fWriter->write(signal);
}
