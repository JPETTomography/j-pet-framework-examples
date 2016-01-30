/**
  *  @copyright Copyright (c) 2014, Wojciech Krzemien
  *  @file TaskB.cpp
  *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
  */

#include "./TaskB.h"
#include "JPetWriter/JPetWriter.h"

//ClassImp(TaskB);

TaskB::TaskB(const char * name, const char * description):
  JPetTask(name, description)
{
}

void TaskB::init(const JPetTaskInterface::Options& opts)
{
  getStatistics().createHistogram( new TH1F("single threshold multiplicity",
					    "Single threshold multipicity within single time window",
					    10, -0.5, 9.5)
				   );
}


void TaskB::exec()
{
  auto tslotRaw = (JPetTimeWindow&) (*getEvent());

  // get number of SigCh-s in a tslot
  auto nSigChs = tslotRaw.getNumberOfSigCh();
  // create an object for the CALIBRATED tslot
  JPetTimeWindow tslotCal;
  tslotCal.setIndex(tslotRaw.getIndex());

  // we would like to check if a signal from one DAQ channel can occur more than once
  // during one Time Slot, so we will create a map(channel, No. of occurences) to count multiplicities
  std::map<int,int> channelMultiplicities;

  // iterate over SigCh's in the tslot and calibrate their times
  for (int i = 0; i < nSigChs; i++) {
    JPetSigCh sigchRaw = tslotRaw[i];
    JPetSigCh sigchCal = sigchRaw; // start the calibrated SigCh as a copy of the raw one

    // do our multiplicity counting
    channelMultiplicities[sigchRaw.getDAQch()]++;

    // a dummy "calibration" example; real calibration should go here
    float time = sigchRaw.getValue();
    time = time * 1.0 + 0.0;
    // set time after calibration in the new SigCh object
    sigchCal.setValue(time);

    // insert the calibrated SigCh into calibrated TimeWindow
    tslotCal.addCh(sigchCal);
  }
  saveTimeWindow(tslotCal);
  
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

void TaskB::saveTimeWindow( JPetTimeWindow slot)
{
  assert(fWriter);
  fWriter->write(slot);
}
