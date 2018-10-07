/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file TimeWindowCreator.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <Unpacker2/Unpacker2/EventIII.h>
#include "TimeWindowCreatorTools.h"
#include <JPetWriter/JPetWriter.h>
#include "UniversalFileLoader.h"
#include "TimeWindowCreator.h"

using namespace jpet_options_tools;
using namespace std;

/**
 * Constructor
 */
TimeWindowCreator::TimeWindowCreator(const char* name): JPetUserTask(name) {}

/**
 * Destructor
 */
TimeWindowCreator::~TimeWindowCreator() {}

/**
 * Init Time Window Creator
 */
bool TimeWindowCreator::init()
{
  INFO("TimeSlot Creation Started");
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  // Reading values from the user options if available
  // Min allowed signal time
  if (isOptionSet(fParams.getOptions(), kMinTimeParamKey)) {
    fMinTime = getOptionAsFloat(fParams.getOptions(), kMinTimeParamKey);
  } else {
    WARNING(
      Form("No value of the %s parameter provided by the user. Using default value of %lf.",
        kMinTimeParamKey.c_str(), fMinTime
      )
    );
  }
  // Max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey)) {
    fMaxTime = getOptionAsFloat(fParams.getOptions(), kMaxTimeParamKey);
  } else {
    WARNING(
      Form("No value of the %s parameter provided by the user. Using default value of %lf.",
        kMaxTimeParamKey.c_str(), fMaxTime
      )
    );
  }
  // Getting time calibration file from user options
  auto calibFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kTimeCalibFileParamKey)) {
    calibFile = getOptionAsString(fParams.getOptions(), kTimeCalibFileParamKey);
  } else {
    WARNING("No path to the time calibration file was provided in user options.");
  }
  // Getting threshold values file from user options
  auto thresholdFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kThresholdFileParamKey)) {
    thresholdFile = getOptionAsString(fParams.getOptions(), kThresholdFileParamKey);
    fSetTHRValuesFromChannels = false;
  } else {
    WARNING("No path to the file with threshold values was provided in user options.");
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  // Use of Time Calibratin and Thresholds files
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibration = UniversalFileLoader::loadConfigurationParameters(calibFile, tombMap);
  if (fTimeCalibration.empty()) {
    ERROR("Time Calibration seems to be empty");
  }
  fThresholds = UniversalFileLoader::loadConfigurationParameters(thresholdFile, tombMap);
  if (fThresholds.empty()) {
    ERROR("Thresholds values seem to be empty");
  }

  // Reference Detector
  // Take coordinates of the main (irradiated strip) from user parameters
  if (isOptionSet(fParams.getOptions(), kMainStripKey)) {
    fMainStripSet = true;
    int code = getOptionAsInt(fParams.getOptions(), kMainStripKey);
    fMainStrip.first = code / 100;  // layer number
    fMainStrip.second = code % 100; // strip number

    INFO(
      Form("Filtering of SigCh-s was requested. Only data from strip %d in layer %d will be used.",
        fMainStrip.second, fMainStrip.first
      )
    );

    // Build a list of allowed channels
    JPetGeomMapping mapper(getParamBank());
    for (int thr = 1; thr <= 4; ++thr) {
      int tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideA, thr);
      fAllowedChannels.insert(tombNumber);
      tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideB, thr);
      fAllowedChannels.insert(tombNumber);
    }
    // Add all reference detector channels to allowed channels list
    for (int thr = 1; thr <= 4; ++thr) {
      int tombNumber = mapper.getTOMB(4, 1, JPetPM::SideA, thr);
      fAllowedChannels.insert(tombNumber);
      tombNumber = mapper.getTOMB(4, 1, JPetPM::SideB, thr);
      fAllowedChannels.insert(tombNumber);
    }
  }

  // Control histograms
  if (fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

/**
 * Execute Time Window Creator
 */
bool TimeWindowCreator::exec()
{
  if (auto event = dynamic_cast <EventIII* const > (fEvent)) {
    int kTDCChannels = event->GetTotalNTDCChannels();
    if (fSaveControlHistos){
      getStatistics().getHisto1D("sig_ch_per_time_slot")->Fill(kTDCChannels);
    }
    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i) {
      auto tdcChannel = dynamic_cast <TDCChannel* const > (tdcChannels->At(i));
      auto tombNumber =  tdcChannel->GetChannel();
      // Skip trigger signals from TRB - every 65th
      if (tombNumber % 65 == 0) continue;
      // Check if channel exists in database from loaded local file
      if (getParamBank().getTOMBChannels().count(tombNumber) == 0) {
        WARNING(
          Form("DAQ Channel %d appears in data but does not exist in the detector setup.", tombNumber)
        );
        continue;
      }
      // Get channel for corresponding number
      JPetTOMBChannel& tombChannel = getParamBank().getTOMBChannel(tombNumber);

      // Reference Detector
      // Ignore irrelevant channels
      if (!filter(tombChannel)) continue;

      auto allSigChs = TimeWindowCreatorTools::buildSigChs(
        tdcChannel, tombChannel, fMaxTime, fMinTime,
        fTimeCalibration, fThresholds, fSetTHRValuesFromChannels,
        getStatistics(), fSaveControlHistos
      );

      auto flaggedSigChs = TimeWindowCreatorTools::flagSigChs(
        allSigChs, getParamBank(), kNumOfThresholds, getStatistics(), fSaveControlHistos
      );

      saveSigChs(flaggedSigChs);
    }
    fCurrEventNumber++;
  } else { return false; }
  return true;
}

/**
 * Terminate Time Window Creator
 */
bool TimeWindowCreator::terminate()
{
  INFO("TimeSlot Creation Ended");
  return true;
}

/**
 * Saving method
 */
void TimeWindowCreator::saveSigChs(const vector<JPetSigCh>& sigChVec)
{
  for (auto & sigCh : sigChVec) { fOutputEvents->add<JPetSigCh>(sigCh); }
}

/**
 * Reference Detector
 * Returns true if signal from the channel given as argument should be passed
 */
bool TimeWindowCreator::filter(const JPetTOMBChannel& channel) const
{
  // If main strip was not defined, pass all channels
  if (!fMainStripSet) return true;
  if (fAllowedChannels.find(channel.getChannel()) != fAllowedChannels.end()) {
    return true;
  }
  return false;
}

/**
 * Init histograms
 */
void TimeWindowCreator::initialiseHistograms(){

  getStatistics().createHistogram(
    new TH1F(
      "sig_ch_per_time_slot", "Signal Channels Per Time Slot",
      250, -0.5, 999.5
    )
  );
  getStatistics().getHisto1D("sig_ch_per_time_slot")
    ->GetXaxis()->SetTitle("Signal Channels in Time Slot");
  getStatistics().getHisto1D("sig_ch_per_time_slot")
    ->GetYaxis()->SetTitle("Number of Time Slots");

  for(int i=1; i<=kNumOfThresholds; i++){
    getStatistics().createHistogram(
      new TH1F(
        Form("pm_occupation_thr%d", i),
        Form("Signal Channels per PM on THR %d", i),
        385, 0.5, 385.5
      )
    );
    getStatistics().getHisto1D(Form("pm_occupation_thr%d", i))
      ->GetXaxis()->SetTitle("PM ID)");
    getStatistics().getHisto1D(Form("pm_occupation_thr%d", i))
      ->GetYaxis()->SetTitle("Number of Signal Channels");
  }

  getStatistics().createHistogram(
    new TH1F("good_vs_bad_sigch", "Number of good and corrupted Events created", 2, 0.5, 2.5)
  );
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(1,"GOOD");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(2,"CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(
    new TH1F("LL_per_PM", "Number of LL found on PMs", 385, 0.5, 385.5));
  getStatistics().getHisto1D("LL_per_PM")
    ->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("LL_per_PM")
    ->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(
    new TH1F("LL_per_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5));
  getStatistics().getHisto1D("LL_per_THR")
    ->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("LL_per_THR")
    ->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(
    new TH1F("LL_time_diff", "Time diff of any LL", 200, 0.0, 300000.0));
  getStatistics().getHisto1D("LL_time_diff")
    ->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("LL_time_diff")
    ->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(
    new TH1F("LT_time_diff", "Any LT time diff", 200, 0.0, 300000.0));
  getStatistics().getHisto1D("LT_time_diff")
    ->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("LT_time_diff")
    ->GetYaxis()->SetTitle("Number of LL pairs");
}
