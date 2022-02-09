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
 *  @file TimeWindowCreator.cpp
 */

#include "TimeWindowCreator.h"
#include "TimeWindowCreatorTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTaskIO/JPetInputHandlerHLD.h>
#include <JPetWriter/JPetWriter.h>
#include <Signals/JPetChannelSignal/JPetChannelSignal.h>
#include <TRandom.h>
#include <boost/property_tree/json_parser.hpp>

#include <iostream>
#include <unpacker_types.hpp>
#include <utility>

using namespace jpet_options_tools;
using namespace std;
namespace pt = boost::property_tree;

TimeWindowCreator::TimeWindowCreator(const char *name) : JPetUserTask(name) {}

TimeWindowCreator::~TimeWindowCreator() {}

bool TimeWindowCreator::init() {
  INFO("TimeSlot Creation Started");
  fOutputEvents = new JPetTimeWindow("JPetChannelSignal");

  // Reading values from the user options if available
  // Min and max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMinTimeParamKey)) {
    fMinTime = getOptionAsDouble(fParams.getOptions(), kMinTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using "
                 "default value of %lf.",
                 kMinTimeParamKey.c_str(), fMinTime));
  }
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey)) {
    fMaxTime = getOptionAsDouble(fParams.getOptions(), kMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using "
                 "default value of %lf.",
                 kMaxTimeParamKey.c_str(), fMaxTime));
  }

  fTimeWindowWidth = fMaxTime - fMinTime;

  // Reading file with offsets to property tree - SiPM calibration per matrix
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey)) {
    pt::read_json(
        getOptionAsString(fParams.getOptions(), kConstantsFileParamKey),
        fConstansTree);
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos =
        getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // build a lookup table of channel offsets
  for (auto &dm : getParamBank().getDataModules()) {
    fChannelOffsets[dm.second->getTBRNetAddress()] =
        dm.second->getChannelsOffset();
  }

  // Control histograms
  if (fSaveControlHistos) {
    initialiseHistograms();
  }
  return true;
}

bool TimeWindowCreator::exec() {
  if (auto event = dynamic_cast<JPetHLDdata *const>(fEvent)) {
    vector<JPetChannelSignal> allChannelSignals;
    unordered_map<int, vector<JPetChannelSignal>> singleChannelSignals;

    for (auto &endp_data : event->fOriginalData) {

      unsigned int address = endp_data.first;
      unsigned int channel_offset = fChannelOffsets.at(address);

      std::vector<unpacker::hit_t> &data = endp_data.second;

      for (auto &hit : data) {

        int channelNumber = channel_offset + hit.channel_id;

        // Skip trigger signals - every 105th modulo 104
        if (channelNumber % 105 == 104) {
          continue;
        }

        if (getParamBank().getChannels().count(channelNumber) == 0) {
          if (fSaveControlHistos) {
            getStatistics().getHisto1D("wrong_channel")->Fill(channelNumber);
          }
        }

        auto &channel = getParamBank().getChannel(channelNumber);
        double offset = fConstansTree.get(
            "channel_offests." + to_string(channel.getID()), 0.0);

        double time = hit.time / 1000.;

        time = time - fTimeWindowWidth;
        time *= -1.;

        if (time < fMinTime || time > fMaxTime) {
          continue;
        }

        auto sigCh = TimeWindowCreatorTools::generateChannelSignal(
            time, channel,
            hit.is_falling_edge == 0 ? JPetChannelSignal::Leading
                                     : JPetChannelSignal::Trailing,
            offset);
        singleChannelSignals[channel.getID()].push_back(sigCh);
      }
    }

    for (auto &sigchs : singleChannelSignals) {
      TimeWindowCreatorTools::flagChannelSignals(sigchs.second, getStatistics(),
                                                 fSaveControlHistos);
      // Sort Signal Channels in time
      TimeWindowCreatorTools::sortByTime(sigchs.second);
      // Save result
      saveChannelSignals(sigchs.second);
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

void TimeWindowCreator::saveChannelSignals(
    const vector<JPetChannelSignal> &channelSigVec) {
  if (channelSigVec.size() > 0) {
    if (fSaveControlHistos) {
      getStatistics().fillHistogram("chsig_tslot", channelSigVec.size());
    }

    double lastTime = 0.0;

    for (auto &channelSig : channelSigVec) {
      if (channelSig.getRecoFlag() == JPetChannelSignal::Good) {
        fOutputEvents->add<JPetChannelSignal>(channelSig);
      }

      if (fSaveControlHistos) {
        if (gRandom->Uniform() < fScalingFactor) {
          getStatistics().fillHistogram("occ_channels",
                                        channelSig.getChannel().getID());

          if (channelSig.getRecoFlag() == JPetRecoSignal::Good) {
            getStatistics().fillHistogram("reco_flags_chsig", 1);
          } else if (channelSig.getRecoFlag() == JPetRecoSignal::Corrupted) {
            getStatistics().fillHistogram("reco_flags_chsig", 2);
          } else if (channelSig.getRecoFlag() == JPetRecoSignal::Unknown) {
            getStatistics().fillHistogram("reco_flags_chsig", 3);
          }
        }

        if (channelSig.getEdgeType() == JPetChannelSignal::Leading &&
            channelSig.getChannel().getThresholdNumber() == 1) {
          if (lastTime != 0.0) {
            getStatistics().fillHistogram("consec_lead_THR1",
                                          channelSig.getTime() - lastTime);
          } else {
            lastTime = channelSig.getTime();
          }
        }
      }
    }
  }
}

void TimeWindowCreator::initialiseHistograms() {
  getStatistics().createHistogramWithAxes(
      new TH1D("chsig_tslot", "Signal Channels Per Time Slot", 50, 0.5, 50.5),
      "Channels Signal in Time Slot", "Number of Time Slots");

  // Channels and PMs IDs from Param Bank
  auto minChannelID = getParamBank().getChannels().begin()->first;
  auto maxChannelID = getParamBank().getChannels().rbegin()->first;

  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().rbegin()->first;

  // Wrong configuration
  getStatistics().createHistogramWithAxes(
      new TH1D("wrong_channels",
               "Channel IDs not found in the json configuration",
               maxChannelID - minChannelID + 1, minChannelID - 0.5,
               maxChannelID + 0.5),
      "Channel ID", "Number of Channel Signals");

  getStatistics().createHistogramWithAxes(
      new TH1D("occ_channels", "Channels occupation (downscaled)",
               maxChannelID - minChannelID + 1, minChannelID - 0.5,
               maxChannelID + 0.5),
      "Channel ID", "Number of Channel Signals");

  getStatistics().createHistogramWithAxes(
      new TH1D("reco_flags_chsig",
               "Number of good and corrupted Channel Sigals created", 4, 0.5,
               4.5),
      " ", "Number of Channel Signals");
  vector<pair<unsigned, string>> binLabels = {
      make_pair(1, "GOOD"), make_pair(2, "CORRUPTED"), make_pair(3, "UNKNOWN"),
      make_pair(4, " ")};
  getStatistics().setHistogramBinLabel(
      "reco_flags_chsig", getStatistics().AxisLabel::kXaxis, binLabels);

  // Flagging histograms
  //  getStatistics().createHistogramWithAxes(new TH1D("filter_LT_tdiff", "LT
  //  time diff", 200, 0.0, 200000.0), "Time Diff [ps]", "Number of LT pairs");
  getStatistics().createHistogramWithAxes(
      new TH1D("filter_LT_tdiff", "LT time diff", 400, -200000.0, 200000.0),
      "Time Diff [ps]", "Number of LT pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_LL_tdiff", "Time diff of LL pairs", 200, 0.0, 5000.0),
      "Time Diff [ps]", "Number of LL pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_TT_tdiff", "Time diff of TT pairs", 200, 0.0, 50000.0),
      "Time Diff [ps]", "Number of TT pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_LL_PM", "Number of LL found on PMs",
               maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5),
      "PM ID", "Number of LL pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_TT_PM", "Number of TT found on PMs",
               maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5),
      "PM ID", "Number of TT pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_LL_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5),
      "THR Number", "Number of LL pairs");

  getStatistics().createHistogramWithAxes(
      new TH1D("filter_TT_THR", "Number of found TT on Thresolds", 4, 0.5, 4.5),
      "THR Number", "Number of TT pairs");

  // Time differences of consecutive lead thr1 SigChs after filtering

  getStatistics().createHistogramWithAxes(
      new TH1D("consec_lead_THR1", "Time diff of consecutive leadings THR1",
               200, 0.0, 50000.0),
      "Time Diff [ps]", "Number of channel signal pairs");
}
