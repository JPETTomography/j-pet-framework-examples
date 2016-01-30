/**
  *  @copyright Copyright (c) 2014, Wojciech Krzemien
  *  @file TaskE.cpp
  *  @author Wojciech Krzemien, wojciech.krzemien@if.uj.edu.pl
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
    if (fHits[0].getSignalA().getTSlotIndex() == currHit.getSignalB().getTSlotIndex()) {
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
