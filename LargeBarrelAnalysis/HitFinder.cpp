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
 *  @file HitFinder.cpp
 */

using namespace std;

#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetWriter/JPetWriter.h>
#include "UniversalFileLoader.h"
#include "HitFinderTools.h"
#include "HitFinder.h"
#include <string>
#include <vector>
#include <map>

using namespace jpet_options_tools;

HitFinder::HitFinder(const char* name) : JPetUserTask(name) { }

bool HitFinder::init()
{
  INFO("Hit finding Started");

  fOutputEvents = new JPetTimeWindow("JPetHit");

  // Reading values from the user options if available
  // Allowed time difference between signals on A and B sides
  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey))
    fABTimeDiff = getOptionAsFloat(fParams.getOptions(), kABTimeDiffParamKey);
  // Getting velocities file from user options
  auto velocitiesFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kVelocityFileParamKey))
    velocitiesFile = getOptionAsString(fParams.getOptions(), kVelocityFileParamKey);
  else
    WARNING("No path to the file with velocities was provided in user options.");
  // Getting number of Reference Detector Scintillator ID
  if (isOptionSet(fParams.getOptions(), kRefDetScinIDParamKey))
    fRefDetScinID = getOptionAsInt(fParams.getOptions(), kRefDetScinIDParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user, indicating that Reference Detector was not used.",
    kRefDetScinIDParamKey.c_str()));
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);

  // Use of velocities file
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fVelocities = UniversalFileLoader::loadConfigurationParameters(velocitiesFile, tombMap);
  if(fVelocities.empty()) ERROR("Velocities map seems to be empty");

  // Control histograms
  if(fSaveControlHistos){
    getStatistics().createHistogram(
      new TH1F("hits_per_time_slot",
        "Number of Hits in Time Window",
        101, -0.5, 100.5));
    getStatistics().getHisto1D("hits_per_time_slot")
      ->GetXaxis()->SetTitle("Hits in Time Slot");
    getStatistics().getHisto1D("hits_per_time_slot")
      ->GetYaxis()->SetTitle("Number of Time Slots");

    getStatistics().createHistogram(
      new TH1F("remain_signals_per_scin",
          "Number of Unused Signals in Scintillator",
          192, 0.5, 192.5));
    getStatistics().getHisto1D("remain_signals_per_scin")
      ->GetXaxis()->SetTitle("ID of Scintillator");
    getStatistics().getHisto1D("remain_signals_per_scin")
      ->GetYaxis()->SetTitle("Number of Unused Signals in Scintillator");

    getStatistics().createHistogram(
      new TH2F("time_diff_per_scin",
        "Signals Time Difference per Scintillator ID",
        200, -2*fABTimeDiff, 2*fABTimeDiff,
        192, 0.5, 192.5));
    getStatistics().getHisto2D("time_diff_per_scin")
      ->GetXaxis()->SetTitle("A-B time difference");
    getStatistics().getHisto2D("time_diff_per_scin")
      ->GetYaxis()->SetTitle("ID of Scintillator");

    getStatistics().createHistogram(
      new TH2F("hit_pos_per_scin",
        "Hit Position per Scintillator ID",
        200, -50.0, 50.0,
        192, 0.5, 192.5));
    getStatistics().getHisto2D("hit_pos_per_scin")
      ->GetXaxis()->SetTitle("Hit z position [cm]");
    getStatistics().getHisto2D("hit_pos_per_scin")
      ->GetYaxis()->SetTitle("ID of Scintillator");
  }
  return true;
}

bool HitFinder::exec()
{
  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    map<int, vector<JPetPhysSignal>> signalSlotMap = HitFinderTools::getSignalsSlotMap(timeWindow);
    vector<JPetHit> allHits = HitFinderTools::matchSignals(getStatistics(), signalSlotMap,
      fVelocities, fABTimeDiff, fRefDetScinID, fSaveControlHistos);
    if(fSaveControlHistos)
      getStatistics().getHisto1D("hits_per_time_slot")->Fill(allHits.size());
    saveHits(allHits);
  } else return false;
  return true;
}

bool HitFinder::terminate()
{
  INFO("Hit finding ended");
  return true;
}

void HitFinder::saveHits(const std::vector<JPetHit>& hits)
{
  auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);
  for (const auto& hit : sortedHits) fOutputEvents->add<JPetHit>(hit);
}
