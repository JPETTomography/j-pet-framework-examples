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
 *  @file TimeWindowCreator.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <Unpacker2/Unpacker2/EventIII.h>
#include <JPetWriter/JPetWriter.h>
#include "UniversalFileLoader.h"
#include "TimeWindowCreator.h"

using namespace jpet_options_tools;

TimeWindowCreator::TimeWindowCreator(const char* name): JPetUserTask(name) {}

bool TimeWindowCreator::init()
{
  INFO("TimeSlot Creation Started");
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  // Reading values from the user options if available
  // Min allowed signal time
  if (isOptionSet(fParams.getOptions(), kMinTimeParamKey))
    fMinTime = getOptionAsFloat(fParams.getOptions(), kMinTimeParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kMinTimeParamKey.c_str(), fMinTime));
  // Max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey))
    fMaxTime = getOptionAsFloat(fParams.getOptions(), kMaxTimeParamKey);
  else
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kMaxTimeParamKey.c_str(), fMaxTime));
  // Getting time calibration file from user options
  auto calibFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kTimeCalibFileParamKey))
    calibFile = getOptionAsString(fParams.getOptions(), kTimeCalibFileParamKey);
  else
    WARNING("No path to the time calibration file was provided in user options.");
  // Getting threshold values file from user options
  auto thresholdFile = std::string("dummyCalibration.txt");
  if (isOptionSet(fParams.getOptions(), kThresholdFileParamKey))
    thresholdFile = getOptionAsString(fParams.getOptions(), kThresholdFileParamKey);
  else
    WARNING("No path to the file with threshold values was provided in user options.");
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);

  // Use of Time Calibratin and Thresholds files
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibration = UniversalFileLoader::loadConfigurationParameters(calibFile, tombMap);
  if (fTimeCalibration.empty()) ERROR("Time Calibration seems to be empty");
  fThresholds = UniversalFileLoader::loadConfigurationParameters(thresholdFile, tombMap);
  if (fThresholds.empty()) ERROR("Thresholds values seem to be empty");

  // Reference Detector
  // Take coordinates of the main (irradiated strip) from user parameters
  if (isOptionSet(fParams.getOptions(), kMainStripKey)) {
    fMainStripSet = true;
    int code = getOptionAsInt(fParams.getOptions(), kMainStripKey);
    fMainStrip.first = code / 100;  // layer number
    fMainStrip.second = code % 100; // strip number

    INFO(Form("Filtering of SigCh-s was requested. Only data from strip %d in layer %d will be used.",
	      fMainStrip.second, fMainStrip.first));

    // build a list of allowed channels
    JPetGeomMapping mapper(getParamBank());
    for(int thr=1;thr<=4;++thr){
      int tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideA, thr);
      fAllowedChannels.insert(tombNumber);
      tombNumber = mapper.getTOMB(fMainStrip.first, fMainStrip.second, JPetPM::SideB, thr);
      fAllowedChannels.insert(tombNumber);
    }
    // Add all reference detector channels to allowed channels list
    for(int thr=1;thr<=4;++thr){
      int tombNumber = mapper.getTOMB(4, 1, JPetPM::SideA, thr);
      fAllowedChannels.insert(tombNumber);
      tombNumber = mapper.getTOMB(4, 1, JPetPM::SideB, thr);
      fAllowedChannels.insert(tombNumber);
    }
  }

  // Control histogram
  if(fSaveControlHistos){
    getStatistics().createHistogram(
      new TH1F("sig_ch_per_time_slot",
        "Signal Channels Per Time Slot",
        250, -0.5, 999.5));
    getStatistics().getHisto1D("sig_ch_per_time_slot")
      ->GetXaxis()->SetTitle("Signal Channels in Time Slot");
    getStatistics().getHisto1D("sig_ch_per_time_slot")
      ->GetYaxis()->SetTitle("Number of Time Slots");
  }
  return true;
}

TimeWindowCreator::~TimeWindowCreator() {}

bool TimeWindowCreator::exec() {
  if (auto event = dynamic_cast <EventIII * const > (fEvent)) {
    int kTDCChannels = event->GetTotalNTDCChannels();
    if(fSaveControlHistos)
      getStatistics().getHisto1D("sig_ch_per_time_slot")->Fill(kTDCChannels);

    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i) {
      auto tdcChannel = dynamic_cast <TDCChannel* const > (tdcChannels->At(i));
      auto tombNumber =  tdcChannel->GetChannel();
      // Skip trigger signals from TRB - every 65th
      if (tombNumber % 65 == 0) continue;
      // Check if channel exists in database from loaded local file
      if ( getParamBank().getTOMBChannels().count(tombNumber) == 0) {
        WARNING(Form("DAQ Channel %d appears in data but does not exist in the setup from DB.", tombNumber));
        continue;
      }
      // Get channel for corresponding number
      JPetTOMBChannel& tombChannel = getParamBank().getTOMBChannel(tombNumber);

      // Reference Detector
      // Ignore irrelevant channels
      if (!filter(tombChannel)) continue;

      // Loop over all Signal Channels in current TOMBChannel
      const int kSignalChannels = tdcChannel->GetHitsNum();
      for (int j = 0; j < kSignalChannels; ++j) {

        // Check for unreasonable times
        // The times should be negative (measured w.r.t end of time window)
        // and not smaller than -1*timeWindowWidth (which can vary for different)
        // data but shoudl not exceed 1 ms, i.e. 1.e6 ns)
        if(tdcChannel->GetLeadTime(j) > fMaxTime ||
             tdcChannel->GetLeadTime(j) < fMinTime ) continue;
        if(tdcChannel->GetTrailTime(j) > fMaxTime ||
             tdcChannel->GetTrailTime(j) < fMinTime ) continue;

        // Create Signal Channels for leading and trailing edge
        JPetSigCh sigChTmpLead = generateSigCh(tombChannel, JPetSigCh::Leading);
        JPetSigCh sigChTmpTrail = generateSigCh(tombChannel, JPetSigCh::Trailing);

        // Set the times in ps [raw times are in ns]
        // and add Time Calibration [also in ns]
        sigChTmpLead.setValue(
          1000.*(tdcChannel->GetLeadTime(j)
            +UniversalFileLoader::getConfigurationParameter(
              fTimeCalibration, sigChTmpLead.getTOMBChannel().getChannel())));
        sigChTmpTrail.setValue(
          1000.*(tdcChannel->GetTrailTime(j)
            +UniversalFileLoader::getConfigurationParameter(
              fTimeCalibration, sigChTmpTrail.getTOMBChannel().getChannel())));

        // Save created Signal Channels
	      fOutputEvents->add<JPetSigCh>(sigChTmpLead);
	      fOutputEvents->add<JPetSigCh>(sigChTmpTrail);
      }
    }
    fCurrEventNumber++;
  }else{
    return false;
  }
  return true;
}

bool TimeWindowCreator::terminate() {
  INFO("TimeSlot Creation Ended");
  return true;
}

/**
 * Sets up Signal Channel fields
 */
JPetSigCh TimeWindowCreator::generateSigCh(
  const JPetTOMBChannel& channel,
  JPetSigCh::EdgeType edge) const {
  JPetSigCh sigCh;
  sigCh.setDAQch(channel.getChannel());
  sigCh.setType(edge);
  sigCh.setThresholdNumber(channel.getLocalChannelNumber());
  sigCh.setPM(channel.getPM());
  sigCh.setFEB(channel.getFEB());
  sigCh.setTRB(channel.getTRB());
  sigCh.setTOMBChannel(channel);
  sigCh.setThreshold(
    UniversalFileLoader::getConfigurationParameter(fThresholds, channel.getChannel()));
  return sigCh;
}

/**
 * Reference Detector
 * Returns true if signal from the channel given as argument should be passed
 */
bool TimeWindowCreator::filter(const JPetTOMBChannel& channel) const{
  // If main strip was not defined, pass all channels
  if(!fMainStripSet) return true;
  if(fAllowedChannels.find(channel.getChannel())
    != fAllowedChannels.end()) return true;
  return false;
}
