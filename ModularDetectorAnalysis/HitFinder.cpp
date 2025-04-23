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

#include "HitFinder.h"
#include "../CommonTools/ToTEnergyConverter.h"
#include "HitFinderTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <string>
#include <vector>

using namespace jpet_options_tools;

HitFinder::HitFinder(const char* name) : JPetUserTask(name) {}

HitFinder::~HitFinder() {}

bool HitFinder::init()
{
  INFO("Hit finding Started");
  fOutputEvents = new JPetTimeWindow("JPetPhysRecoHit");

  // Reading values from the user options if available
  // Getting bools for saving control and calibration histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kSaveCalibHistosParamKey))
  {
    fSaveCalibHistos = getOptionAsBool(fParams.getOptions(), kSaveCalibHistosParamKey);
  }

  // Reading file with effective light velocit and TOF synchronization constants to property tree
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  if (isOptionSet(fParams.getOptions(), kMinHitMultiDiffParamKey))
  {
    fMinHitMultiplicity = getOptionAsInt(fParams.getOptions(), kMinHitMultiDiffParamKey);
    INFO(Form("Saving only hits with multiplicity %d or grater", fMinHitMultiplicity));
  }

  // Allowed time difference between signals on A and B sides
  if (isOptionSet(fParams.getOptions(), kABTimeDiffParamKey))
  {
    fABTimeDiff = getOptionAsDouble(fParams.getOptions(), kABTimeDiffParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kABTimeDiffParamKey.c_str(), fABTimeDiff));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
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

  // Control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  return true;
}

bool HitFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    auto signalsBySlot = HitFinderTools::getSignalsByScin(timeWindow);
    auto totConverter = fToTConverterFactory.getEnergyConverter();
    auto allHits =
        HitFinderTools::matchAllSignals(signalsBySlot, fABTimeDiff, fConstansTree, fConvertToT, totConverter, getStatistics(), fSaveControlHistos);
    if (allHits.size() > 0)
    {
      saveHits(allHits);
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool HitFinder::terminate()
{
  INFO("Hit finding ended");
  return true;
}

void HitFinder::saveHits(const std::vector<JPetPhysRecoHit>& hits)
{
  auto sortedHits = hits;
  HitFinderTools::sortByTime(sortedHits);

  if (fSaveControlHistos)
  {
    getStatistics().fillHistogram("hits_tslot", hits.size());
  }

  for (auto& hit : sortedHits)
  {
    // Checking minimal multiplicity condition
    int multi = hit.getSignalA().getPMSignals().size() + hit.getSignalB().getPMSignals().size();
    if (fMinHitMultiplicity != -1 && multi < fMinHitMultiplicity)
    {
      continue;
    }

    fOutputEvents->add<JPetPhysRecoHit>(hit);
    if (fSaveControlHistos)
    {
      int scinID = hit.getScin().getID();
      getStatistics().fillHistogram("hits_scin", scinID, sortedHits.size());
      getStatistics().fillHistogram("hit_pos", hit.getPosZ(), hit.getPosY(), hit.getPosX());
      getStatistics().fillHistogram("hit_z_pos_scin", scinID, hit.getPosZ());
      getStatistics().fillHistogram("hit_multi", multi);
      getStatistics().fillHistogram("hit_multi_scin", scinID, multi);
      getStatistics().fillHistogram("hit_tdiff_scin", scinID, hit.getTimeDiff());

      if (hit.getToT() != 0.0)
      {
        getStatistics().fillHistogram("hit_tot_scin", scinID, hit.getToT());
        getStatistics().fillHistogram("hit_tot_scin_z_pos", scinID, hit.getToT(), hit.getPosZ());
      }
      if (fConvertToT && hit.getEnergy() > 0.0)
      {
        getStatistics().fillHistogram("conv_tot_range", hit.getToT());
        getStatistics().fillHistogram("conv_dep_energy", hit.getEnergy());
        getStatistics().fillHistogram("conv_dep_energy_vs_tot", hit.getEnergy(), hit.getToT());
      }
    }
  }
}

void HitFinder::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  getStatistics().createHistogramWithAxes(new TH1D("hits_tslot", "Number of Hits in Time Window", 400, 0.5, 400.5), "Hits in Time Slot",
                                          "Number of Time Slots");

  getStatistics().createHistogramWithAxes(
      new TH1D("hits_scin", "Number of Hits per Scintillators", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5), "Scin ID",
      "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH3D("hit_pos", "Hit Position", 101, -50.5, 50.5, 101, -50.5, 50.5, 101, -50.5, 50.5), "Z [cm]",
                                          "X [cm]", "Y [cm]");

  getStatistics().createHistogramWithAxes(new TH2D("hit_z_pos_scin", "Z-axis position of hits in Scintillators", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 121, -30.5, 30.5),
                                          "Scintillator ID", "Hit z-pos [cm]");

  getStatistics().createHistogramWithAxes(new TH1D("hit_multi", "Number of signals from SiPMs in created hit", 12, -0.5, 11.5), "Number of Signals",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH2D("hit_multi_scin", "Number of signals from SiPMs in created hit per Scin",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 12, -0.5, 11.5),
                                          "Scintillator ID", "Signal multiplicity [ps]");

  // Time diff and ToT per scin
  getStatistics().createHistogramWithAxes(new TH2D("hit_tdiff_scin", "Hit Time Difference per Scintillator ID", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 201, -1.1 * fABTimeDiff, 1.1 * fABTimeDiff),
                                          "Scintillator ID", "A-B time difference [ps]");

  getStatistics().createHistogramWithAxes(new TH2D("hit_tot_scin", "Hit ToT divided by multiplicity, all hits", maxScinID - minScinID + 1,
                                                   minScinID - 0.5, maxScinID + 0.5, 200, 0.0, 1.2 * fToTHistoUpperLimit),
                                          "Scintillator ID", "Time over Threshold [ps]");

  getStatistics().createHistogramWithAxes(new TH3D("hit_tot_scin_z_pos", "Hit ToT divided by multiplicity per scin vs z-axis",
                                                   maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5, 200, 0.0, 1.2 * fToTHistoUpperLimit,
                                                   121, -30.5, 30.5),
                                          "Scintillator ID", "Time over Threshold [ps]", "z [cm]");

  // Unused sigals stats
  getStatistics().createHistogramWithAxes(
      new TH1D("remain_signals_scin", "Number of Unused Signals in Scintillator", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Unused Signals in Scintillator");

  getStatistics().createHistogramWithAxes(
      new TH1D("remain_signals_tdiff", "Time Diff of an unused signal and the consecutive one", 200, fABTimeDiff, 5.0 * fABTimeDiff),
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
