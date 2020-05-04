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

#include "TimeWindowCreator.h"
#include "EventIII.h"
#include "JPetGeomMapping/JPetGeomMapping.h"
#include "JPetOptionsTools/JPetOptionsTools.h"
#include "JPetWriter/JPetWriter.h"
#include "TimeWindowCreatorTools.h"
#include "UniversalFileLoader.h"

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
    WARNING(Form("No value of the %s parameter provided by the user. Using "
                 "default value of %lf.",
                 kMinTimeParamKey.c_str(), fMinTime));
  }
  // Max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey)) {
    fMaxTime = getOptionAsFloat(fParams.getOptions(), kMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using "
                 "default value of %lf.",
                 kMaxTimeParamKey.c_str(), fMaxTime));
  }
  // Getting time calibration file from user options
  auto calibFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kTimeCalibFileParamKey)) {
    calibFile = getOptionAsString(fParams.getOptions(), kTimeCalibFileParamKey);
  } else {
    WARNING(
        "No path to the time calibration file was provided in user options.");
  }
  // Getting threshold values file from user options
  auto thresholdFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kThresholdFileParamKey)) {
    thresholdFile =
        getOptionAsString(fParams.getOptions(), kThresholdFileParamKey);
    fSetTHRValuesFromChannels = false;
  } else {
    WARNING("No path to the file with threshold values was provided in user "
            "options.");
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos =
        getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  // Use of Time Calibratin and Thresholds files
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibration =
      UniversalFileLoader::loadConfigurationParameters(calibFile, tombMap);
  if (fTimeCalibration.empty()) {
    ERROR("Time Calibration seems to be empty");
  }
  fThresholds =
      UniversalFileLoader::loadConfigurationParameters(thresholdFile, tombMap);
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

    INFO(Form("Filtering of SigCh-s was requested. Only data from strip %d in "
              "layer %d will be used.",
              fMainStrip.second, fMainStrip.first));

    // Build a list of allowed channels
    JPetGeomMapping mapper(getParamBank());
    for (int thr = 1; thr <= 4; ++thr) {
      int tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second,
                                      JPetPM::SideA, thr);
      fAllowedChannels.insert(tombNumber);
      tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second,
                                  JPetPM::SideB, thr);
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
  if (fSaveControlHistos) {
    initialiseHistograms();
  }
  return true;
}

bool TimeWindowCreator::exec() {
  if (auto event = dynamic_cast<EventIII *const>(fEvent)) {
    int kTDCChannels = event->GetTotalNTDCChannels();
    if (fSaveControlHistos) {
      getStatistics().fillHistogram("sig_ch_per_time_slot", kTDCChannels);
    }
    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i) {
      auto tdcChannel = dynamic_cast<TDCChannel *const>(tdcChannels->At(i));
      auto tombNumber = tdcChannel->GetChannel();
      // Skip trigger signals from TRB - every 65th
      if (tombNumber % 65 == 0)
        continue;
      // Check if channel exists in database from loaded local file
      if (getParamBank().getTOMBChannels().count(tombNumber) == 0) {
        WARNING(Form("DAQ Channel %d appears in data but does not exist in the "
                     "detector setup.",
                     tombNumber));
        continue;
      }
      // Get channel for corresponding number
      auto &tombChannel = getParamBank().getTOMBChannel(tombNumber);

      // Reference Detector
      // Ignore irrelevant channels
      if (!isAllowedChannel(tombChannel))
        continue;

      // Building Signal Channels for this TOMB Channel
      auto allSigChs = TimeWindowCreatorTools::buildSigChs(
          tdcChannel, tombChannel, fTimeCalibration, fThresholds, fMaxTime,
          fMinTime, fSetTHRValuesFromChannels, getStatistics(),
          fSaveControlHistos);

      // Sort Signal Channels in time
      TimeWindowCreatorTools::sortByValue(allSigChs);

      // Flag with Good or Corrupted
      TimeWindowCreatorTools::flagSigChs(allSigChs, getStatistics(),
                                         fSaveControlHistos);

      // Save result
      saveSigChs(allSigChs);
    }
    fCurrEventNumber++;
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
  }
}

/**
 * Reference Detector
 * Returns true if signal from the channel given as argument should be passed
 */
bool TimeWindowCreator::isAllowedChannel(JPetTOMBChannel &tombChannel) const {
  // If main strip was not defined, pass all channels
  if (!fMainStripSet)
    return true;
  if (fAllowedChannels.find(tombChannel.getChannel()) !=
      fAllowedChannels.end()) {
    return true;
  }
  return false;
}

void TimeWindowCreator::initialiseHistograms() {
  getStatistics().createHistogramWithAxes(new TH1D("sig_ch_per_time_slot", "Signal Channels Per Time Slot", 250, -0.125, 999.875),
                                                    "Signal Channels in Time Slot", "Number of Time Slots");

  for (int i = 1; i <= kNumOfThresholds; i++) {
    getStatistics().createHistogramWithAxes(new TH1D(Form("pm_occupation_thr%d", i), Form("Signal Channels per PM on THR %d", i), 
                                                    385, 0.5, 385.5), "PM ID)", "Number of Signal Channels");
  }

  getStatistics().createHistogramWithAxes(
    new TH1D("good_vs_bad_sigch", "Number of good and corrupted SigChs created",
                                            3, 0.5, 3.5), "Quality", "Number of SigChs");
  std::vector<std::pair<unsigned, std::string>> binLabels;
  binLabels.push_back(std::make_pair(1,"GOOD"));
  binLabels.push_back(std::make_pair(2,"CORRUPTED"));
  binLabels.push_back(std::make_pair(3,"UNKNOWN"));
  getStatistics().setHistogramBinLabel("good_vs_bad_sigch",
                                       getStatistics().AxisLabel::kXaxis, binLabels);

  getStatistics().createHistogramWithAxes(new TH1D("LT_time_diff", "LT time diff", 200, -250.0, 999750.0),
                                                    "Time Diff [ps]", "Number of LL pairs");
  getStatistics().createHistogramWithAxes(new TH1D("LL_per_PM", "Number of LL found on PMs", 385, 0.5, 385.5),
                                                    "PM ID", "Number of LL pairs");
  getStatistics().createHistogramWithAxes(new TH1D("LL_per_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5),
                                                    "THR Number", "Number of LL pairs");
  getStatistics().createHistogramWithAxes(new TH1D("LL_time_diff", "Time diff of LL pairs", 200, -750.0, 299250.0),
                                                    "Time Diff [ps]", "Number of LL pairs");
  getStatistics().createHistogramWithAxes(new TH1D("TT_per_PM", "Number of TT found on PMs", 385, 0.5, 385.5),
                                                    "PM ID", "Number of TT pairs");
  getStatistics().createHistogramWithAxes(new TH1D("TT_per_THR", "Number of found TT on Thresolds", 4, 0.5, 4.5),
                                                    "THR Number", "Number of TT pairs");
  getStatistics().createHistogramWithAxes(new TH1D("TT_time_diff", "Time diff of TT pairs", 200, -750.0, 299250.0),
                                                    "Time Diff [ps]", "Number of TT pairs");
}
