/**
  *  @copyright Copyright (c) 2014, Wojciech Krzemien
  *  @file TaskA.cpp
  *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
  */

#include "TaskA.h"
#include "JPetWriter/JPetWriter.h"
#include "JPetUnpacker/Unpacker2/Event.h"

//ClassImp(TaskA);

TaskA::TaskA(const char * name, const char * description):
  JPetTask(name, description),
  fCurrEventNumber(0)
{
}


void TaskA::exec()
{
  // Get HLD Event
  auto evt = reinterpret_cast<Event*> (getEvent());
  //evt->GetTitle();
  //evt->GetName();

  // get number of TDC hits in a HLD event
  int ntdc = evt->GetTotalNTDCHits();

  JPetTimeWindow tslot;
  tslot.setIndex(fCurrEventNumber);

  TClonesArray* tdcHits = evt->GetTDCHitsArray();

  // iterate over TDC hits
  TDCHit* tdchit;
  for (int i = 0; i < ntdc; i++) {

    tdchit = (TDCHit*) tdcHits->At(i);
    JPetSigCh sigChTmp;

    // get data channel number which corresponds to the TOMB Channel number
    auto tomb_number =  tdchit->GetChannel();

    if (tomb_number % 65 == 0) { // skip trigger signals from TRB
      continue;
    }

    // find index of corresponding TOMB Channel
    //int tombch_index = 0;
    const auto  kNumberOfTOMBs = getParamBank().getTOMBChannelsSize();
    auto tombch_index = 0;
    for (; tombch_index < kNumberOfTOMBs; tombch_index++) {
       auto currChannel = getParamBank().getTOMBChannel(tombch_index).getChannel();
       if (currChannel == tomb_number) break;
    }

    //while (tombch_index < kNumberOfTOMBs 
           //&& 
           //!= tomb_number)
      //tombch_index++;
    if (tombch_index == getParamBank().getTOMBChannelsSize()) { // TOMBChannel object not found
      WARNING( Form("TOMB Channel for DAQ channel %d was not found in database! Ignoring this channel.", tomb_number) );
      continue;
    }
    // get TOMBChannel object from database
    JPetTOMBChannel& tomb_channel = getParamBank().getTOMBChannel(tombch_index);
    sigChTmp.setDAQch(tomb_number);

    if (tomb_channel.getLocalChannelNumber() % 2 == 1) { // leading edge
      sigChTmp.setType(JPetSigCh::Leading);
      //set the local threshold number(1...4) using local channel number (1...8)
      sigChTmp.setThresholdNumber(
        (tomb_channel.getLocalChannelNumber() + 1) / 2.);
    } else {  // trailing edge
      sigChTmp.setType(JPetSigCh::Trailing);
      //set the local threshold number(1...4) using local channel number (1...8)
      sigChTmp.setThresholdNumber(
        tomb_channel.getLocalChannelNumber() / 2.);
    }

    // store pointers to the related parametric objects
    sigChTmp.setPM(tomb_channel.getPM());
    sigChTmp.setFEB(tomb_channel.getFEB());
    sigChTmp.setTRB(tomb_channel.getTRB());
    sigChTmp.setTOMBChannel(tomb_channel);

    sigChTmp.setThreshold(tomb_channel.getThreshold());

    // finally, set the time in ps
    sigChTmp.setValue(tdchit->GetLeadTime1() * 10.);

    // and add the sigCh to TimeWindow
    tslot.addCh(sigChTmp);
  }
  saveTimeWindow(tslot);

  fCurrEventNumber++;
}

void TaskA::terminate()
{
}



void TaskA::saveTimeWindow( JPetTimeWindow slot)
{
  assert(fWriter);
  fWriter->write(slot);
}
