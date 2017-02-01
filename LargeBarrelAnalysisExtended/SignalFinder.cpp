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
 *  @file SignalFinder.cpp
 */

using namespace std;

#include <map>
#include <string>
#include <vector>
#include <JPetWriter/JPetWriter.h>
#include "SignalFinderTools.h"
#include "SignalFinder.h"



SignalFinder::SignalFinder(const char* name, const char* description, bool saveControlHistos)
  : JPetTask(name, description)
{
  fSaveControlHistos = saveControlHistos;
}

SignalFinder::~SignalFinder() {}

//SignalFinder init method
void SignalFinder::init(const JPetTaskInterface::Options& opts)
{
  INFO("Signal finding started.");

  if (opts.count(fEdgeMaxTimeParamKey)) {
    kSigChEdgeMaxTime = std::atof(opts.at(fEdgeMaxTimeParamKey).c_str());
  } 
  if (opts.count(fLeadTrailMaxTimeParamKey)) {
    kSigChLeadTrailMaxTime = std::atof(opts.at(fLeadTrailMaxTimeParamKey).c_str());
  }

  fBarrelMap.buildMappings(getParamBank());
  if (fSaveControlHistos) {
    getStatistics().createHistogram(new TH1F("remainig_leading_sig_ch_per_thr", "Remainig Leading Signal Channels", 4, 0.5, 4.5));
    getStatistics().createHistogram(new TH1F("remainig_trailing_sig_ch_per_thr", "Remainig Trailing Signal Channels", 4, 0.5, 4.5));
    getStatistics().createHistogram(new TH1F("TOT_thr_1", "TOT on threshold 1 [ns]", 100, 20.0, 100.0));
    getStatistics().createHistogram(new TH1F("TOT_thr_2", "TOT on threshold 2 [ns]", 100, 20.0, 100.0));
    getStatistics().createHistogram(new TH1F("TOT_thr_3", "TOT on threshold 3 [ns]", 100, 20.0, 100.0));
    getStatistics().createHistogram(new TH1F("TOT_thr_4", "TOT on threshold 4 [ns]", 100, 20.0, 100.0));
  }
}

//SignalFinder execution method
void SignalFinder::exec()
{

  //getting the data from event in propriate format
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(getEvent())) {

    //mapping method invocation
    map<int, vector<JPetSigCh>> sigChsPMMap = SignalFinderTools::getSigChsPMMapById(timeWindow);
    std::cout <<"after getSigChsPMMapById" <<std::endl;
    //building signals method invocation
    vector<JPetRawSignal> allSignals = SignalFinderTools::buildAllSignals(timeWindow->getIndex(), sigChsPMMap, kNumOfThresholds , getStatistics(), fSaveControlHistos,  kSigChEdgeMaxTime, kSigChLeadTrailMaxTime);

    std::cout <<"after buildAllSignals" <<std::endl;
    //saving method invocation
    saveRawSignals(allSignals);
  }
}

//SignalFinder finish method
void SignalFinder::terminate()
{
  INFO("Signal finding ended.");
}


//saving method
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& sigChVec)
{
  assert(fWriter);
  for (const auto & sigCh : sigChVec) {
    fWriter->write(sigCh);
  }
}

//other methods - TODO check if neccessary
void SignalFinder::setWriter(JPetWriter* writer)
{
  fWriter = writer;
}

void SignalFinder::setParamManager(JPetParamManager* paramManager)
{
  fParamManager = paramManager;
}

const JPetParamBank& SignalFinder::getParamBank() const
{
  return fParamManager->getParamBank();
}
