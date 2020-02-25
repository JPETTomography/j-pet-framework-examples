/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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

#include "JPetOptionsTools/JPetOptionsTools.h"
#include "TimeWindowCreatorTools.h"
#include "JPetWriter/JPetWriter.h"
#include "UniversalFileLoader.h"
#include "TimeWindowCreator.h"
#include "EventIII.h"

using namespace jpet_options_tools;
using namespace std;

TimeWindowCreator::TimeWindowCreator(const char *name) : JPetUserTask(name) {}

TimeWindowCreator::~TimeWindowCreator() {}

bool TimeWindowCreator::init() {
  INFO("TimeSlot Creation Started");
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  // Reading values from the user options if available
  // Min allowed signal time
  if (isOptionSet(fParams.getOptions(), kMinTimeParamKey)) {
    fMinTime = getOptionAsFloat(fParams.getOptions(), kMinTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kMinTimeParamKey.c_str(), fMinTime
    ));
  }
  // Max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey)) {
    fMaxTime = getOptionAsFloat(fParams.getOptions(), kMaxTimeParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kMaxTimeParamKey.c_str(), fMaxTime
    ));
  }
  // Getting time calibration file from user options
  auto calibFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kTimeCalibFileParamKey)) {
    calibFile = getOptionAsString(fParams.getOptions(), kTimeCalibFileParamKey);
  } else {
    WARNING(
      "No path to the time calibration file was provided in user options."
    );
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos =
        getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  // Use of Time Calibratin and Thresholds files
  fTimeCalibration = UniversalFileLoader::loadConfigurationParameters(calibFile, getParamBank());
  if (fTimeCalibration.empty()) {
    ERROR("Time Calibration seems to be empty");
  }

  // Control histograms
  if (fSaveControlHistos) {
    initialiseHistograms();
  }
  return true;
}

bool TimeWindowCreator::exec() {
  if (auto event = dynamic_cast<EventIII *const>(fEvent)) {
    int kTDCChannels = event->GetTotalNTDCChannels();
    if (fSaveControlHistos) {
      getStatistics().getHisto1D("sig_ch_per_time_slot")->Fill(kTDCChannels);
    }
    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i) {
      auto tdcChannel = dynamic_cast<TDCChannel *const>(tdcChannels->At(i));
      auto channelID = tdcChannel->GetChannel();

      // Skip trigger signals from TRB - every 65th
      if (channelID % 65 == 0) continue;

      // Check if channel exists in database from loaded local file
      if (getParamBank().getChannels().count(channelID) == 0) {
        WARNING(Form(
          "DAQ Channel %d appears in data but does not exist in the detector setup.", channelID)
        );
        continue;
      }
      // Get channel for corresponding number
      auto& channel = getParamBank().getChannel(channelID);

      // Building Signal Channels for this Channel
      auto allSigChs = TimeWindowCreatorTools::buildSigChs(
        tdcChannel, channel, fTimeCalibration, fMaxTime, fMinTime,
        getStatistics(), fSaveControlHistos
      );

      // Sort Signal Channels in time
      TimeWindowCreatorTools::sortByTime(allSigChs);

      // Flag with Good or Corrupted
      TimeWindowCreatorTools::flagSigChs(
        allSigChs, getStatistics(), fSaveControlHistos
      );

      // Save result
      saveSigChs(allSigChs);
    }
  } else {
    return false;
  }
  return true;
}

bool TimeWindowCreator::terminate() {
  INFO("TimeSlot Creation Ended");
  return true;
}

void TimeWindowCreator::saveSigChs(const vector<JPetSigCh> &sigChVec) {
  for (auto &sigCh : sigChVec) {
    fOutputEvents->add<JPetSigCh>(sigCh);
    if(fSaveControlHistos){
      getStatistics().getHisto1D("pm_occ")->Fill(sigCh.getChannel().getPM().getID());
      getStatistics().getHisto1D(Form("pm_occ_thr%d", sigCh.getChannel().getThresholdNumber()))
      ->Fill(sigCh.getChannel().getPM().getID());
    }
  }
}

void TimeWindowCreator::initialiseHistograms() {

  getStatistics().createHistogram(new TH1F(
    "sig_ch_per_time_slot", "Signal Channels Per Time Slot",
    250, -0.5, 999.5
  ));
  getStatistics().getHisto1D("sig_ch_per_time_slot")->GetXaxis()->SetTitle(
    "Signal Channels in Time Slot"
  );
  getStatistics().getHisto1D("sig_ch_per_time_slot")->GetYaxis()->SetTitle(
    "Number of Time Slots"
  );

  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().end()->first;

  getStatistics().createHistogram(new TH1F(
    "pm_occ", "Signal Channels per PM", maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("pm_occ")->GetXaxis()->SetTitle("PM ID)");
  getStatistics().getHisto1D("pm_occ")->GetYaxis()->SetTitle(
    "Number of Signal Channels"
  );

  for (int i = 1; i <= kNumOfThresholds; i++) {
    getStatistics().createHistogram(new TH1F(
      Form("pm_occ_thr%d", i),
      Form("Signal Channels per PM on THR %d", i),
      maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
    ));
    getStatistics().getHisto1D(Form("pm_occ_thr%d", i))->GetXaxis()->SetTitle("PM ID)");
    getStatistics().getHisto1D(Form("pm_occ_thr%d", i))->GetYaxis()->SetTitle(
      "Number of Signal Channels"
    );
  }

  // Filtering
  getStatistics().createHistogram(new TH1F(
    "good_vs_bad_sigch", "Number of good and corrupted SigChs created",
    3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(
    new TH1F("LT_time_diff", "LT time diff", 200, 0.0, 100000.0)
  );
  getStatistics().getHisto1D("LT_time_diff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("LT_time_diff")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "LL_per_PM", "Number of LL found on PMs", maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("LL_per_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("LL_per_PM")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "LL_per_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("LL_per_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("LL_per_THR")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "LL_time_diff", "Time diff of LL pairs", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("LL_time_diff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("LL_time_diff")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "TT_per_PM", "Number of TT found on PMs", maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("TT_per_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("TT_per_PM")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "TT_per_THR", "Number of found TT on Thresolds", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("TT_per_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("TT_per_THR")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "TT_time_diff", "Time diff of TT pairs", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("TT_time_diff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("TT_time_diff")->GetYaxis()->SetTitle("Number of TT pairs");
}
