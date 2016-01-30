/**
  *  @copyright Copyright (c) 2014, Wojciech Krzemien
  *  @file TaskC3.cpp
  *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
  */

#include "./TaskC3.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskC3);

TaskC3::TaskC3(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskC3::init(const JPetTaskInterface::Options& opts)
{
}


void TaskC3::exec()
{
  // A dummy analysis example:
  auto currSignal = (JPetRecoSignal&) (*getEvent());
  savePhysSignal(createPhysSignal(currSignal));
}

void TaskC3::terminate()
{
}

JPetPhysSignal TaskC3::createPhysSignal(JPetRecoSignal& recoSignal)
{
  // create a Phys Signal
  JPetPhysSignal physSignal;

  // use the values from Reco Signal to set the physical properties of signal
  // here, a dummy example - much more sophisticated procedures should go here
  physSignal.setPhe( recoSignal.getCharge() * 1.0 + 0.0 );
  physSignal.setQualityOfPhe(1.0);

  // in the previous module (C2) we have reconstructed one time at arbitrary
  // threshold - now we retrieve it by getting a map of times vs. thresholds,
  // and taking its first (and only) element by the begin() iterator. We get
  // an std::pair, where first is the threshold value, and second is time.
  double time = recoSignal.getRecoTimesAtThreshold().begin()->second;
  physSignal.setTime(time);
  physSignal.setQualityOfTime(1.0);

  // store the original JPetRecoSignal in the PhysSignal as a processing history
  physSignal.setRecoSignal(recoSignal);
  return physSignal;
}

void TaskC3::savePhysSignal( JPetPhysSignal sig)
{
  assert(fWriter);
  fWriter->write(sig);
}
