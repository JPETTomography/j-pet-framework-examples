/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
#include <JPetWriter/JPetWriter.h>
#include "TimeWindowCreator.h"
#include <EventIII.h>

using namespace jpet_options_tools;
using namespace std;

TimeWindowCreator::TimeWindowCreator(const char* name): JPetUserTask(name) {}

TimeWindowCreator::~TimeWindowCreator() {}

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
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Control histograms
  if (fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool TimeWindowCreator::exec()
{
  if (auto event = dynamic_cast<EventIII* const> (fEvent)) {
    int kTDCChannels = event->GetTotalNTDCChannels();
    if (fSaveControlHistos){
      getStatistics().getHisto1D("sig_ch_per_time_slot")->Fill(kTDCChannels);
    }
    // Loop over all TDC channels in file
    auto tdcChannels = event->GetTDCChannelsArray();
    for (int i = 0; i < kTDCChannels; ++i) {
      auto tdcChannel = dynamic_cast<TDCChannel* const> (tdcChannels->At(i));
      auto channelNumber = tdcChannel->GetChannel();

      // Skip trigger signals - every 105th
      if (channelNumber % 105 == 0) continue;

      // Check if channel exists in database from loaded local file
      if (getParamBank().getChannels().count(channelNumber) == 0) {
        if (fSaveControlHistos){
          getStatistics().getHisto1D("channel_missing")->Fill(channelNumber);
        }
        WARNING(
          Form("DAQ Channel %d appears in data but does not exist in the detector setup.", channelNumber)
        );
        continue;
      }

      // Get channel for corresponding number
      auto& channel = getParamBank().getChannel(channelNumber);

      vector<JPetSigCh> allSigChs;

      // Loop over all entries on leading edge in current TDCChannel and create SigCh
      for (int j = 0; j < tdcChannel->GetLeadHitsNum(); j++) {
        auto leadTime = tdcChannel->GetLeadTime(j);
        if (leadTime > fMaxTime || leadTime < fMinTime ) { continue; }
        auto leadSigCh = generateSigCh(
          leadTime, channel, JPetSigCh::Leading
        );
        allSigChs.push_back(leadSigCh);
        if (fSaveControlHistos){
          fillChannelHistos(channel, JPetSigCh::Leading);
        }
      }

      // Loop over all entries on trailing edge in current TDCChannel and create SigCh
      for (int j = 0; j < tdcChannel->GetTrailHitsNum(); j++) {
        auto trailTime = tdcChannel->GetTrailTime(j);
        if (trailTime > fMaxTime || trailTime < fMinTime ) { continue; }
        auto trailSigCh = generateSigCh(
          trailTime, channel, JPetSigCh::Trailing
        );
        allSigChs.push_back(trailSigCh);
        if (fSaveControlHistos){
          fillChannelHistos(channel, JPetSigCh::Trailing);
        }
      }
      // Save result
      saveSigChs(allSigChs);
    }
    fCurrEventNumber++;
  } else { return false; }
  return true;
}

/**
* Sets up Signal Channel fields
*/
void TimeWindowCreator::fillChannelHistos(
  const JPetChannel& channel, JPetSigCh::EdgeType edge
) {
  getStatistics().getHisto1D("channel_occ")->Fill(channel.getID());
  getStatistics().getHisto1D("channel_thrnum")->Fill(channel.getThresholdNumber());
  getStatistics().getHisto1D("matrix_occ")->Fill(channel.getPM().getMatrixPosition());
  getStatistics().getHisto1D("pm_occ")->Fill(channel.getPM().getID());
  getStatistics().getHisto1D("scin_occ")->Fill(channel.getPM().getScin().getID());
  getStatistics().getHisto1D("slot_occ")->Fill(channel.getPM().getScin().getID());

  if(edge == JPetSigCh::Leading){
    getStatistics().getHisto1D("channel_occ_leads")->Fill(channel.getID());
    getStatistics().getHisto1D("channel_thrnum_leads")->Fill(channel.getThresholdNumber());
    getStatistics().getHisto1D("matrix_occ_leads")->Fill(channel.getPM().getMatrixPosition());
    getStatistics().getHisto1D("pm_occ_leads")->Fill(channel.getPM().getID());
    getStatistics().getHisto1D("scin_occ_leads")->Fill(channel.getPM().getScin().getID());
    getStatistics().getHisto1D("slot_occ_leads")->Fill(channel.getPM().getScin().getID());
  } else if(edge == JPetSigCh::Trailing){
    getStatistics().getHisto1D("channel_occ_trails")->Fill(channel.getID());
    getStatistics().getHisto1D("channel_thrnum_trails")->Fill(channel.getThresholdNumber());
    getStatistics().getHisto1D("matrix_occ_trails")->Fill(channel.getPM().getMatrixPosition());
    getStatistics().getHisto1D("pm_occ_trails")->Fill(channel.getPM().getID());
    getStatistics().getHisto1D("scin_occ_trails")->Fill(channel.getPM().getScin().getID());
    getStatistics().getHisto1D("slot_occ_trails")->Fill(channel.getPM().getScin().getID());
  }

  if(channel.getPM().getSide() == JPetPM::SideA){
    getStatistics().getHisto1D("pm_occ_sides")->Fill(1);
  }else if(channel.getPM().getSide() == JPetPM::SideB){
    getStatistics().getHisto1D("pm_occ_sides")->Fill(2);
  }
}

/**
* Sets up Signal Channel fields
*/
JPetSigCh TimeWindowCreator::generateSigCh(
  double time, const JPetChannel& channel, JPetSigCh::EdgeType edge
) {
  JPetSigCh sigCh;
  sigCh.setTime(1000.*time);
  sigCh.setType(edge);
  sigCh.setChannel(channel);
  sigCh.setRecoFlag(JPetSigCh::Good);
  return sigCh;
}

bool TimeWindowCreator::terminate()
{
  INFO("TimeSlot Creation Ended");
  return true;
}

void TimeWindowCreator::saveSigChs(const vector<JPetSigCh>& sigChVec)
{
  for (auto & sigCh : sigChVec) { fOutputEvents->add<JPetSigCh>(sigCh); }
}

void TimeWindowCreator::initialiseHistograms(){

  getStatistics().createHistogram(
    new TH1F("sig_ch_per_time_slot", "Signal Channels Per Time Slot", 50, -0.5, 50.5)
  );
  getStatistics().getHisto1D("sig_ch_per_time_slot")
  ->GetXaxis()->SetTitle("Signal Channels in Time Slot");
  getStatistics().getHisto1D("sig_ch_per_time_slot")
  ->GetYaxis()->SetTitle("Number of Time Slots");

  // Channels
  getStatistics().createHistogram(
    new TH1F("channel_occ", "Channels occupation", 211, 2099.5, 2310.5)
  );
  getStatistics().getHisto1D("channel_occ")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("channel_occ")->GetYaxis()->SetTitle("Number of SigCh");

  // 2204 2309
  // high occ: 2198, 2199, 2204, 2205, 2296, 2303, 2304, 2305, 2308, 2309
  getStatistics().createHistogram(
    new TH1F("channel_missing", "Channels missing in configuration", 211, 2099.5, 2310.5)
  );
  getStatistics().getHisto1D("channel_missing")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("channel_missing")->GetYaxis()->SetTitle("Counts");

  getStatistics().createHistogram(
    new TH1F("channel_thrnum", "Channels threshold numbers", 4, 0.5, 4.5)
  );
  getStatistics().getHisto1D("channel_thrnum")->GetXaxis()->SetTitle("Channel Threshold Number");
  getStatistics().getHisto1D("channel_thrnum")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("channel_thrnum_leads", "Channels threshold numbers LEADS", 4, 0.5, 4.5)
  );
  getStatistics().getHisto1D("channel_thrnum_leads")->GetXaxis()->SetTitle("Channel Threshold Number");
  getStatistics().getHisto1D("channel_thrnum_leads")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("channel_thrnum_trails", "Channels threshold numbers TRAILS", 4, 0.5, 4.5)
  );
  getStatistics().getHisto1D("channel_thrnum_trails")->GetXaxis()->SetTitle("Channel Threshold Number");
  getStatistics().getHisto1D("channel_thrnum_trails")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("channel_occ_leads", "Channels occupation - Leading channels", 211, 2099.5, 2310.5)
  );
  getStatistics().getHisto1D("channel_occ_leads")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("channel_occ_leads")->GetYaxis()->SetTitle("Number of Lading SigCh");

  getStatistics().createHistogram(
    new TH1F("channel_occ_trails", "Channels occupation - Trailing channels", 211, 2099.5, 2310.5)
  );
  getStatistics().getHisto1D("channel_occ_trails")->GetXaxis()->SetTitle("Channel ID");
  getStatistics().getHisto1D("channel_occ_trails")->GetYaxis()->SetTitle("Number of Trailing SigCh");

  // SiPMs
  getStatistics().createHistogram(
    new TH1F("pm_occ", "PMs occupation", 111, 399.5, 510.5)
  );
  getStatistics().getHisto1D("pm_occ")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("pm_occ")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("pm_occ_leads", "PMs occupation LEADS", 111, 399.5, 510.5)
  );
  getStatistics().getHisto1D("pm_occ_leads")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("pm_occ_leads")->GetYaxis()->SetTitle("Number of Leading SigCh");

  getStatistics().createHistogram(
    new TH1F("pm_occ_trails", "PMs occupation TRAILS", 111, 399.5, 510.5)
  );
  getStatistics().getHisto1D("pm_occ_trails")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("pm_occ_trails")->GetYaxis()->SetTitle("Number of Trailing SigCh");

  getStatistics().createHistogram(
    new TH1F("pm_occ_sides", "PMs occupation of sides", 2, 0.5, 2.5)
  );
  getStatistics().getHisto1D("pm_occ_sides")->GetXaxis()->SetBinLabel(1, "SIDE A");
  getStatistics().getHisto1D("pm_occ_sides")->GetXaxis()->SetBinLabel(2, "SIDE B");
  getStatistics().getHisto1D("pm_occ_sides")->GetYaxis()->SetTitle("Number of SigCh");

  // Matrix position
  getStatistics().createHistogram(
    new TH1F("matrix_occ", "Position in matrix in PMs occupation", 5, -0.5, 4.5)
  );
  getStatistics().getHisto1D("matrix_occ")->GetXaxis()->SetTitle("Matrix position");
  getStatistics().getHisto1D("matrix_occ")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("matrix_occ_leads", "Position in matrix in PMs occupation LEADS", 5, -0.5, 4.5)
  );
  getStatistics().getHisto1D("matrix_occ_leads")->GetXaxis()->SetTitle("Matrix position");
  getStatistics().getHisto1D("matrix_occ_leads")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("matrix_occ_trails", "Position in matrix in PMs occupation TRAILS", 5, -0.5, 4.5)
  );
  getStatistics().getHisto1D("matrix_occ_trails")->GetXaxis()->SetTitle("Matrix position");
  getStatistics().getHisto1D("matrix_occ_trails")->GetYaxis()->SetTitle("Number of SigCh");

  // Scins
  getStatistics().createHistogram(
    new TH1F("scin_occ", "Scins occupation", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("scin_occ")->GetXaxis()->SetTitle("SCIN ID");
  getStatistics().getHisto1D("scin_occ")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("scin_occ_leads", "Scins occupation LEADS",16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("scin_occ_leads")->GetXaxis()->SetTitle("SCIN ID");
  getStatistics().getHisto1D("scin_occ_leads")->GetYaxis()->SetTitle("Number of Leading SigCh");

  getStatistics().createHistogram(
    new TH1F("scin_occ_trails", "Scins occupation TRAILS", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("scin_occ_trails")->GetXaxis()->SetTitle("SCIN ID");
  getStatistics().getHisto1D("scin_occ_trails")->GetYaxis()->SetTitle("Number of Trailing SigCh");

  // Slots
  getStatistics().createHistogram(
    new TH1F("slot_occ", "Slots occupation", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("slot_occ")->GetXaxis()->SetTitle("SLOT ID");
  getStatistics().getHisto1D("slot_occ")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("slot_occ_leads", "Slots occupation LEADS", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("slot_occ_leads")->GetXaxis()->SetTitle("SLOT ID");
  getStatistics().getHisto1D("slot_occ_leads")->GetYaxis()->SetTitle("Number of Leading SigCh");

  getStatistics().createHistogram(
    new TH1F("slot_occ_trails", "Slots occupation TRAILS", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("slot_occ_trails")->GetXaxis()->SetTitle("SLOT ID");
  getStatistics().getHisto1D("slot_occ_trails")->GetYaxis()->SetTitle("Number of Trailing SigCh");
}
