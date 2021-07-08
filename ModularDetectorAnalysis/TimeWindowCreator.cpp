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

#include <boost/property_tree/json_parser.hpp>

#include <EventIII.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include <TRandom.h>

#include "TimeWindowCreator.h"
#include "TimeWindowCreatorTools.h"

using namespace jpet_options_tools;
using namespace std;
namespace pt = boost::property_tree;

TimeWindowCreator::TimeWindowCreator(const char* name) : JPetUserTask(name) {}

TimeWindowCreator::~TimeWindowCreator() {}

bool TimeWindowCreator::init()
{
  INFO("TimeSlot Creation Started");
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  // Reading values from the user options if available
  // Min and max allowed signal time
  if (isOptionSet(fParams.getOptions(), kMinTimeParamKey))
  {
    fMinTime = getOptionAsDouble(fParams.getOptions(), kMinTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMinTimeParamKey.c_str(), fMinTime));
  }
  if (isOptionSet(fParams.getOptions(), kMaxTimeParamKey))
  {
    fMaxTime = getOptionAsDouble(fParams.getOptions(), kMaxTimeParamKey);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeParamKey.c_str(), fMaxTime));
  }

  // Reading file with offsets to property tree - SiPM calibration per matrix
  if (isOptionSet(fParams.getOptions(), kConstantsFileParamKey))
  {
    pt::read_json(getOptionAsString(fParams.getOptions(), kConstantsFileParamKey), fConstansTree);
  }

  if (isOptionSet(fParams.getOptions(), kMaskedChannlesParamKey))
  {
    fMaskedChannels = getOptionAsVectorOfInts(fParams.getOptions(), kMaskedChannlesParamKey);
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Control histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

bool TimeWindowCreator::exec()
{
  if (auto event = dynamic_cast<EventIII* const>(fEvent))
  {
    int kTDCChannels = event->GetTotalNTDCChannels();

    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i)
    {
      auto tdcChannel = dynamic_cast<TDCChannel* const>(tdcChannels->At(i));
      auto channelNumber = tdcChannel->GetChannel();

      // Skip trigger signals - every 105th modulo 104
      if (channelNumber % 105 == 104)
      {
        continue;
      }

      // Skip masked channels
      if (find(fMaskedChannels.begin(), fMaskedChannels.end(), channelNumber) != fMaskedChannels.end())
      {
        continue;
      }

      // Check if channel exists in database from loaded local json file
      if (getParamBank().getChannels().count(channelNumber) == 0)
      {
        if (fSaveControlHistos)
        {
          getStatistics().getHisto1D("wrong_channel")->Fill(channelNumber);
        }
        continue;
      }

      // Get channel for corresponding number
      auto& channel = getParamBank().getChannel(channelNumber);

      // Building Signal Channels for this Channel
      auto allSigChs = TimeWindowCreatorTools::buildSigChs(tdcChannel, channel, fMaxTime, fMinTime, fConstansTree);

      // Sort Signal Channels in time
      TimeWindowCreatorTools::sortByTime(allSigChs);

      // Flag with Good or Corrupted
      TimeWindowCreatorTools::flagSigChs(allSigChs, getStatistics(), fSaveControlHistos);

      // Save result
      saveSigChs(allSigChs);
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool TimeWindowCreator::terminate()
{
  INFO("TimeSlot Creation Ended");
  return true;
}

void TimeWindowCreator::saveSigChs(const vector<JPetSigCh>& sigChVec)
{
  if (sigChVec.size() > 0)
  {
    if (fSaveControlHistos)
    {
      getStatistics().getHisto1D("sigch_tslot")->Fill(sigChVec.size());
    }

    double lastTime = 0.0;

    for (auto& sigCh : sigChVec)
    {
      if (sigCh.getRecoFlag() == JPetSigCh::Good)
      {
        fOutputEvents->add<JPetSigCh>(sigCh);
      }

      if (fSaveControlHistos)
      {
        if (gRandom->Uniform() < fScalingFactor)
        {
          getStatistics().getHisto1D("channel_occ")->Fill(sigCh.getChannel().getID());
          if (sigCh.getRecoFlag() == JPetSigCh::Good)
          {
            getStatistics().getHisto1D("filter_sigch")->Fill(1);
          }
          else if (sigCh.getRecoFlag() == JPetSigCh::Corrupted)
          {
            getStatistics().getHisto1D("filter_sigch")->Fill(2);
          }
          else if (sigCh.getRecoFlag() == JPetSigCh::Unknown)
          {
            getStatistics().getHisto1D("filter_sigch")->Fill(3);
          }
        }

        if (sigCh.getType() == JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber() == 1)
        {
          if (lastTime != 0.0)
          {
            getStatistics().getHisto1D("consec_lead_THR1")->Fill(sigCh.getTime() - lastTime);
          }
          else
          {
            lastTime = sigCh.getTime();
          }
        }
      }
    }
  }
}

void TimeWindowCreator::initialiseHistograms()
{

  getStatistics().createHistogram(new TH1F("sigch_tslot", "Signal Channels Per Time Slot", 50, 0.5, 50.5));
  getStatistics().getHisto1D("sigch_tslot")->GetXaxis()->SetTitle("Signal Channels in Time Slot");
  getStatistics().getHisto1D("sigch_tslot")->GetYaxis()->SetTitle("Number of Time Slots");

  // Channels and PMs IDs from Param Bank
  auto minChannelID = getParamBank().getChannels().begin()->first;
  auto maxChannelID = getParamBank().getChannels().rbegin()->first;

  auto minPMID = getParamBank().getPMs().begin()->first;
  auto maxPMID = getParamBank().getPMs().rbegin()->first;

  // Wrong configuration
  getStatistics().createHistogram(new TH1F("wrong_channel", "Channel IDs not found in the json configuration", maxChannelID - minChannelID + 1,
                                           minChannelID - 0.5, maxChannelID + 0.5));
  getStatistics().getHisto1D("wrong_channel")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("wrong_channel")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
      new TH1F("channel_occ", "Channels occupation (downscaled)", maxChannelID - minChannelID + 1, minChannelID - 0.5, maxChannelID + 0.5));
  getStatistics().getHisto1D("channel_occ")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("channel_occ")->GetYaxis()->SetTitle("Number of SigCh");

  // Flagging histograms
  getStatistics().createHistogram(new TH1F("filter_sigch", "Number of good and corrupted SigChs created", 3, 0.5, 3.5));
  getStatistics().getHisto1D("filter_sigch")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("filter_sigch")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("filter_sigch")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("filter_sigch")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F("filter_LT_tdiff", "LT time diff", 200, 0.0, 200000.0));
  getStatistics().getHisto1D("filter_LT_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_LT_tdiff")->GetYaxis()->SetTitle("Number of LT pairs");

  getStatistics().createHistogram(new TH1F("filter_LL_PM", "Number of LL found on PMs", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5));
  getStatistics().getHisto1D("filter_LL_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("filter_LL_PM")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F("filter_LL_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5));
  getStatistics().getHisto1D("filter_LL_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("filter_LL_THR")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F("filter_LL_tdiff", "Time diff of LL pairs", 200, 0.0, 5000.0));
  getStatistics().getHisto1D("filter_LL_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_LL_tdiff")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F("filter_TT_PM", "Number of TT found on PMs", maxPMID - minPMID + 1, minPMID - 0.5, maxPMID + 0.5));
  getStatistics().getHisto1D("filter_TT_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("filter_TT_PM")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F("filter_TT_THR", "Number of found TT on Thresolds", 4, 0.5, 4.5));
  getStatistics().getHisto1D("filter_TT_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("filter_TT_THR")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F("filter_TT_tdiff", "Time diff of TT pairs", 200, 0.0, 50000.0));
  getStatistics().getHisto1D("filter_TT_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_TT_tdiff")->GetYaxis()->SetTitle("Number of TT pairs");

  // Time differences of consecutive lead thr1 SigChs after filtering
  getStatistics().createHistogram(new TH1F("consec_lead_THR1", "Time diff of consecutive leadings THR1", 200, 0.0, 50000.0));
  getStatistics().getHisto1D("consec_lead_THR1")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("consec_lead_THR1")->GetYaxis()->SetTitle("Number of SigCh pairs");
}
