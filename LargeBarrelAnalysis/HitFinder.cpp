/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>

#include "HitFinder.h"
#include "HitFinderTools.h"
#include "ToTEnergyConverterFactory.h"
#include "UniversalFileLoader.h"

#include <map>
#include <string>
#include <vector>

using namespace tot_energy_converter;
using namespace jpet_options_tools;

HitFinder::HitFinder(const char* name) : JPetUserTask(name) {}

HitFinder::~HitFinder() {}

bool HitFinder::init()
{
  INFO("Hit finding Started");
  fOutputEvents = new JPetTimeWindow("JPetHit");

  // Reading values from the user options if available
  // Getting bool for using bad signals
  if (isOptionSet(fParams.getOptions(), kUseCorruptedSignalsParamKey))
  {
    fUseCorruptedSignals = getOptionAsBool(fParams.getOptions(), kUseCorruptedSignalsParamKey);
    if (fUseCorruptedSignals)
    {
      WARNING("Hit Finder is using Corrupted Signals, as set by the user");
    }
    else
    {
      WARNING("Hit Finder is NOT using Corrupted Signals, as set by the user");
    }
  }
  else
  {
    WARNING("Hit Finder is not using Corrupted Signals (default option)");
  }
  // Allowed time difference between signals on A and B sides
  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey))
  {
    fABTimeDiff = getOptionAsFloat(fParams.getOptions(), kABTimeDiffParamKey);
  }
  // Getting velocities file from user options
  auto velocitiesFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kVelocityFileParamKey))
  {
    velocitiesFile = getOptionAsString(fParams.getOptions(), kVelocityFileParamKey);
  }
  else
  {
    WARNING("No path to the file with velocities was provided in user options.");
  }
  // Getting number of Reference Detector Scintillator ID
  if (isOptionSet(fParams.getOptions(), kRefDetScinIDParamKey))
  {
    fRefDetScinID = getOptionAsInt(fParams.getOptions(), kRefDetScinIDParamKey);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user, indicating that Reference Detector was not used.", kRefDetScinIDParamKey.c_str()));
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Use of velocities file
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fVelocities = UniversalFileLoader::loadConfigurationParameters(velocitiesFile, tombMap);
  if (fVelocities.empty())
  {
    ERROR("Velocities map seems to be empty");
  }

  // Loading parameters for conversion to ToT to energy
  if (isOptionSet(fParams.getOptions(), kConvertToTParamKey))
  {
    fConvertToT = getOptionAsBool(fParams.getOptions(), kConvertToTParamKey);
    if (fConvertToT)
    {
      INFO("Hit finder performs conversion of ToT to deposited energy with provided params.");
      fToTConverterFactory.loadConverterOptions(fParams.getOptions());
    }
    else
    {
      INFO("Hit finder will not convert ToT to deposited energy since no user parameters are provided.");
    }
  }

  if (isOptionSet(fParams.getOptions(), kTOTCalculationType))
  {
    fTOTCalculationType = getOptionAsString(fParams.getOptions(), kTOTCalculationType);
  }
  else
  {
    WARNING("No TOT calculation option given by the user. Using standard sum.");
  }

  // Control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool HitFinder::exec()
{
  if (auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    auto signalsBySlot = HitFinderTools::getSignalsBySlot(timeWindow, fUseCorruptedSignals);
    auto totConverter = fToTConverterFactory.getEnergyConverter();
    auto allHits = HitFinderTools::matchAllSignals(signalsBySlot, fVelocities, fABTimeDiff, fRefDetScinID, fConvertToT, totConverter, getStatistics(),
                                                   fSaveControlHistos);
    if (fSaveControlHistos)
    {
      getStatistics().fillHistogram("hits_per_time_slot", allHits.size());
    }
    saveHits(allHits);
  }
  else
    return false;
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
  for (const auto& hit : sortedHits)
  {
    if (fSaveControlHistos)
    {
      auto tot = HitFinderTools::calculateTOT(hit, HitFinderTools::getTOTCalculationType(fTOTCalculationType));
      getStatistics().fillHistogram("TOT_all_hits", tot);
      if (hit.getRecoFlag() == JPetHit::Good)
      {
        getStatistics().fillHistogram("TOT_good_hits", tot);
      }
      else if (hit.getRecoFlag() == JPetHit::Corrupted)
      {
        getStatistics().fillHistogram("TOT_corr_hits", tot);
      }
    }
    fOutputEvents->add<JPetHit>(hit);
  }
}

void HitFinder::initialiseHistograms()
{

  getStatistics().createHistogramWithAxes(new TH1D("good_vs_bad_hits", "Number of good and corrupted Hits created", 3, 0.5, 3.5), "Quality",
                                          "Number of Hits");

  std::vector<std::pair<unsigned, std::string>> binLabels;
  binLabels.push_back(std::make_pair(1, "GOOD"));
  binLabels.push_back(std::make_pair(2, "CORRUPTED"));
  binLabels.push_back(std::make_pair(3, "UNKNOWN"));
  getStatistics().setHistogramBinLabel("good_vs_bad_hits", getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(new TH1D("hits_per_time_slot", "Number of Hits in Time Window", 100, -0.5, 99.5), "Hits in Time Slot",
                                          "Number of Time Slots");

  getStatistics().createHistogramWithAxes(new TH2D("time_diff_per_scin", "Signals Time Difference per Scintillator ID", 4 * fABTimeDiff / 10,
                                                   -2 * fABTimeDiff, 2 * fABTimeDiff, 192, 0.5, 192.5),
                                          "A-B time difference", "ID of Scintillator");

  getStatistics().createHistogramWithAxes(new TH2D("hit_pos_per_scin", "Hit Position per Scintillator ID", 200, -49.75, 50.25, 192, 0.5, 192.5),
                                          "Hit z position [cm]", "ID of Scintillator");

  // TOT calculating for all hits and reco flags
  getStatistics().createHistogramWithAxes(new TH1D("TOT_all_hits", "TOT of all hits", 200, -250.0, 99750.0), "Time over Threshold [ps]",
                                          "Number of Hits");
  getStatistics().createHistogramWithAxes(new TH1D("TOT_good_hits", "TOT of hits with GOOD flag", 200, -250.0, 99750.0), "Time over Threshold [ps]",
                                          "Number of Hits");
  getStatistics().createHistogramWithAxes(new TH1D("TOT_corr_hits", "TOT of hits with CORRUPTED flag", 200, -250.0, 99750.0),
                                          "Time over Threshold [ps]", "Number of Hits");
  getStatistics().createHistogramWithAxes(new TH1D("remain_signals_per_scin", "Number of Unused Signals in Scintillator", 192, 0.5, 192.5),
                                          "ID of Scintillator", "Number of Unused Signals in Scintillator");
  getStatistics().createHistogramWithAxes(
      new TH1D("remain_signals_tdiff", "Time Diff of an unused signal and the consecutive one", 200, fABTimeDiff - 125.0, 49875.0 + fABTimeDiff),
      "Time difference [ps]", "Number of Signals");

  if (fConvertToT)
  {
    auto converterRange = fToTConverterFactory.getEnergyConverter().getRange();
    auto totConverter = fToTConverterFactory.getEnergyConverter();

    auto minToT = converterRange.first;
    auto maxToT = converterRange.second;
    auto minEDep = totConverter(converterRange.first);
    auto maxEDep = totConverter(converterRange.second);

    getStatistics().createHistogramWithAxes(new TH1D("conv_tot_range", "TOT of hits in range of conversion function", 200, minToT, maxToT),
                                            "Time over Threshold [ps]", "Number of Hits");
    getStatistics().createHistogramWithAxes(
        new TH1D("conv_dep_energy", "Deposited energy of hits, converted from ToT with provied formula", 200, minEDep, maxEDep),
        "Deposited energy [keV]", "Number of Hits");
    getStatistics().createHistogramWithAxes(new TH2D("conv_dep_energy_vs_tot",
                                                     "Deposited energy of hits, converted from ToT with provied formula vs. input ToT", 200, minEDep,
                                                     maxEDep, 200, minToT, maxToT),
                                            "Deposited energy [keV]", "ToT of Hit [ps]");
  }
}
