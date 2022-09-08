/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleHitFinder.cpp
 */

#include "RedModuleHitFinder.h"
#include "../ModularDetectorAnalysis/HitFinderTools.h"
#include "RedModuleHitFinderTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <boost/property_tree/json_parser.hpp>
#include <map>
#include <string>
#include <vector>

using namespace jpet_options_tools;
using namespace std;

RedModuleHitFinder::RedModuleHitFinder(const char* name) : JPetUserTask(name) {}

RedModuleHitFinder::~RedModuleHitFinder() {}

bool RedModuleHitFinder::init()
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

  // Reading WLS config file
  if (isOptionSet(fParams.getOptions(), kWLSConfigFileParamKey))
  {
    boost::property_tree::read_json(getOptionAsString(fParams.getOptions(), kWLSConfigFileParamKey), fWLSConfigTree);
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

  if (isOptionSet(fParams.getOptions(), kWLSScinTimeDiffParamKey))
  {
    fWLSScinTimeDiff = getOptionAsDouble(fParams.getOptions(), kWLSScinTimeDiffParamKey);
  }
  else
  {
    WARNING(
        Form("No value of the %s parameter provided by the user. Using default value of %lf.", kWLSScinTimeDiffParamKey.c_str(), fWLSScinTimeDiff));
  }

  if (isOptionSet(fParams.getOptions(), kTimeDiffOffsetParamKey))
  {
    fTimeDiffOffset = getOptionAsDouble(fParams.getOptions(), kTimeDiffOffsetParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kTimeDiffOffsetParamKey.c_str(), fTimeDiffOffset));
  }

  // For plotting ToT histograms
  if (isOptionSet(fParams.getOptions(), kToTHistoUpperLimitParamKey))
  {
    fToTHistoUpperLimit = getOptionAsDouble(fParams.getOptions(), kToTHistoUpperLimitParamKey);
  }

  // Control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }

  return true;
}

bool RedModuleHitFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    auto allSigals = HitFinderTools::getSignalsByScin(timeWindow);

    // Copying map for signals only from matrices attached to the scintillators (241-279)
    map<int, vector<JPetMatrixSignal>> wlsSignals;
    map<int, vector<JPetMatrixSignal>> redSignals;
    map<int, vector<JPetMatrixSignal>> refSignals;

    auto minWLSScinID = 201;
    auto maxWLSScinID = 240;
    auto minRedScinID = 241;
    auto maxRedScinID = 266;
    auto minRefScinID = 267;
    auto maxRefkScinID = 279;

    // Separating signals registered on WLS layer
    copy_if(begin(allSigals), end(allSigals), inserter(wlsSignals, begin(wlsSignals)),
            [&minWLSScinID, &maxWLSScinID](pair<int, vector<JPetMatrixSignal>> p) { return p.first >= minWLSScinID && p.first <= maxWLSScinID; });

    // Separating signals registered on Red Module
    copy_if(begin(allSigals), end(allSigals), inserter(redSignals, begin(redSignals)),
            [&minRedScinID, &maxRedScinID](pair<int, vector<JPetMatrixSignal>> p) { return p.first >= minRedScinID && p.first <= maxRedScinID; });

    // Separating signals registered on Black Module (reference)
    copy_if(begin(allSigals), end(allSigals), inserter(refSignals, begin(refSignals)),
            [&minRefScinID, &maxRefkScinID](pair<int, vector<JPetMatrixSignal>> p) { return p.first >= minRefScinID && p.first <= maxRefkScinID; });

    auto redHits = HitFinderTools::matchAllSignals(redSignals, fABTimeDiff, fConstansTree, getStatistics(), fSaveControlHistos);

    auto refHits = HitFinderTools::matchAllSignals(refSignals, fABTimeDiff, fConstansTree, getStatistics(), fSaveControlHistos);

    auto wlsHits = RedModuleHitFinderTools::matchHitsWithWLSSignals(redHits, wlsSignals, fWLSScinTimeDiff, fTimeDiffOffset, fConstansTree,
                                                                    fWLSConfigTree, getStatistics(), fSaveControlHistos);

    if (fSaveControlHistos)
    {
      getStatistics().fillHistogram("hits_tslot", refHits.size() + wlsHits.size());
      getStatistics().fillHistogram("hits_red_tslot", redHits.size());
      getStatistics().fillHistogram("hits_ref_tslot", refHits.size());
      getStatistics().fillHistogram("hits_wls_tslot", wlsHits.size());
      fillHistograms(redHits);
      fillHistograms(refHits);
      fillHistograms(wlsHits);
    }

    // Hits from red module are merged with WLS, so no need to save them twice
    saveHits(refHits);
    saveHits(wlsHits);
  }
  else
  {
    return false;
  }
  return true;
}

bool RedModuleHitFinder::terminate()
{
  INFO("Hit finding ended");
  return true;
}

void RedModuleHitFinder::fillHistograms(const vector<JPetPhysRecoHit>& hits)
{
  for (auto& hit : hits)
  {
    int scinID = hit.getScin().getID();
    int multi = hit.getSignalA().getPMSignals().size() + hit.getSignalB().getPMSignals().size();

    getStatistics().fillHistogram("hits_scin", scinID);
    getStatistics().fillHistogram("hit_pos", hit.getPosZ(), hit.getPosX(), hit.getPosY());
    getStatistics().fillHistogram("hit_z_pos_scin", scinID, hit.getPosZ());
    getStatistics().fillHistogram("hit_multi", multi);
    getStatistics().fillHistogram("hit_multi_scin", scinID, multi);
    getStatistics().fillHistogram("hit_tdiff_scin", scinID, hit.getTimeDiff());

    if (hit.getToT() != 0.0)
    {
      getStatistics().fillHistogram("hit_tot_scin", scinID, hit.getToT());
    }
  }
}

void RedModuleHitFinder::saveHits(const vector<JPetPhysRecoHit>& hits)
{
  for (auto& hit : hits)
  {
    // Save hit
    fOutputEvents->add<JPetPhysRecoHit>(hit);
  }
}

void RedModuleHitFinder::initialiseHistograms()
{
  auto minScinID = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  getStatistics().createHistogramWithAxes(new TH1D("hits_tslot", "Number of Hits in Time Window", 60, 0.5, 60.5), "Hits in Time Slot",
                                          "Number of Time Slots");

  getStatistics().createHistogramWithAxes(new TH1D("hits_wls_tslot", "Number of WLS Hits in Time Window", 60, 0.5, 60.5), "Hits in Time Slot",
                                          "Number of Time Slots");

  getStatistics().createHistogramWithAxes(new TH1D("hits_red_tslot", "Number of Red Module Hits in Time Window", 60, 0.5, 60.5), "Hits in Time Slot",
                                          "Number of Time Slots");

  getStatistics().createHistogramWithAxes(new TH1D("hits_ref_tslot", "Number of Reference Module Hits in Time Window", 60, 0.5, 60.5),
                                          "Hits in Time Slot", "Number of Time Slots");

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

  // WLS - scintilator time differences for calibration
  getStatistics().createHistogramWithAxes(new TH3D("hit_scin_wls_tdiff", "Coincidences between scintillator and WLS hits", 26, 240.5, 266.5, 40,
                                                   200.5, 240.5, 200, -fWLSScinTimeDiff, fWLSScinTimeDiff),
                                          "Scin ID", "WLS ID", "Time difference [ps]");
  // Unused sigals stats
  getStatistics().createHistogramWithAxes(
      new TH1D("remain_signals_scin", "Number of Unused Signals in Scintillator", maxScinID - minScinID + 1, minScinID - 0.5, maxScinID + 0.5),
      "Scintillator ID", "Number of Unused Signals in Scintillator");

  getStatistics().createHistogramWithAxes(
      new TH1D("remain_signals_tdiff", "Time Diff of an unused signal and the consecutive one", 200, fABTimeDiff, 5.0 * fABTimeDiff),
      "Time difference [ps]", "Number of Signals");
}
