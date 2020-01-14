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

#include <JPetOptionsTools/JPetOptionsTools.h>
#include "TimeWindowCreatorTools.h"
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
        WARNING(
          Form("DAQ Channel %d appears in data but does not exist in the detector setup.", channelNumber)
        );
        continue;
      }

      // Get channel for corresponding number
      auto& channel = getParamBank().getChannel(channelNumber);

      // Building Signal Channels for this TOMB Channel
      auto allSigChs = TimeWindowCreatorTools::buildSigChs(
        tdcChannel, channel, fMaxTime, fMinTime
      );

      // Sort Signal Channels in time
      TimeWindowCreatorTools::sortByTime(allSigChs);

      // Flag with Good or Corrupted
      TimeWindowCreatorTools::flagSigChs(allSigChs, getStatistics(), fSaveControlHistos);

      // Save result
      saveSigChs(allSigChs);
    }
    fCurrEventNumber++;
  } else { return false; }
  return true;
}

bool TimeWindowCreator::terminate()
{
  INFO("TimeSlot Creation Ended");
  return true;
}

void TimeWindowCreator::saveSigChs(const vector<JPetSigCh>& sigChVec)
{
  if(sigChVec.size()>0){
    double lastTime = 0.0;

    for (auto & sigCh : sigChVec) {
      if(sigCh.getRecoFlag()==JPetSigCh::Good){
        fOutputEvents->add<JPetSigCh>(sigCh);

        if(fSaveControlHistos){
          getStatistics().getHisto1D("channel_occ")->Fill(sigCh.getChannel().getID());
          getStatistics().getHisto1D("channel_thrnum")->Fill(sigCh.getChannel().getThresholdNumber());
          getStatistics().getHisto1D("matrix_occ")->Fill(sigCh.getChannel().getPM().getMatrixPosition());
          getStatistics().getHisto1D("pm_occ")->Fill(sigCh.getChannel().getPM().getID());
          getStatistics().getHisto1D("scin_occ")->Fill(sigCh.getChannel().getPM().getScin().getID());

          if(sigCh.getChannel().getPM().getSide()==JPetPM::SideA){
            getStatistics().getHisto1D("pm_occ_sides")->Fill(2);
            if(sigCh.getChannel().getPM().getScin().getID()>200
            && sigCh.getChannel().getPM().getScin().getID()<214){
              getStatistics().getHisto1D(Form("mtx_%d_A", sigCh.getChannel().getPM().getScin().getID()))
              ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
            } else {
              WARNING(Form("Sth wrong, scin ID: %d, PM ID: %d, Channel ID: %d",
                sigCh.getChannel().getPM().getScin().getID(),
                sigCh.getChannel().getPM().getID(),
                sigCh.getChannel().getID())
              );
            }

          } else if(sigCh.getChannel().getPM().getSide()==JPetPM::SideB){
            getStatistics().getHisto1D("pm_occ_sides")->Fill(3);
            if(sigCh.getChannel().getPM().getScin().getID()>200
            && sigCh.getChannel().getPM().getScin().getID()<214){
              getStatistics().getHisto1D(Form("mtx_%d_B", sigCh.getChannel().getPM().getScin().getID()))
              ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
            } else {
              WARNING(Form("Sth wrong, scin ID: %d, PM ID: %d, Channel ID: %d",
                sigCh.getChannel().getPM().getScin().getID(),
                sigCh.getChannel().getPM().getID(),
                sigCh.getChannel().getID())
              );
            }
          }

          if(sigCh.getType() == JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber()==1){
            if(lastTime != 0.0){
              getStatistics().getHisto1D("consec_lead_THR1")->Fill(sigCh.getTime()-lastTime);
            } else{
              lastTime = sigCh.getTime();
            }
          }
        }
      }
    }
  }
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

  getStatistics().createHistogram(
    new TH1F("channel_thrnum", "Channels threshold numbers", 4, 0.5, 4.5)
  );
  getStatistics().getHisto1D("channel_thrnum")->GetXaxis()->SetTitle("Channel Threshold Number");
  getStatistics().getHisto1D("channel_thrnum")->GetYaxis()->SetTitle("Number of SigCh");

  // SiPMs
  getStatistics().createHistogram(
    new TH1F("pm_occ", "PMs occupation", 111, 399.5, 510.5)
  );
  getStatistics().getHisto1D("pm_occ")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("pm_occ")->GetYaxis()->SetTitle("Number of SigCh");

  getStatistics().createHistogram(
    new TH1F("pm_occ_sides", "PMs occupation of sides", 3, 0.5, 3.5)
  );
  getStatistics().getHisto1D("pm_occ_sides")->GetXaxis()->SetBinLabel(2, "SIDE A");
  getStatistics().getHisto1D("pm_occ_sides")->GetXaxis()->SetBinLabel(3, "SIDE B");
  getStatistics().getHisto1D("pm_occ_sides")->GetYaxis()->SetTitle("Number of SigCh");

  // Matrix position
  getStatistics().createHistogram(
    new TH1F("matrix_occ", "Position in matrix in PMs occupation", 5, -0.5, 4.5)
  );
  getStatistics().getHisto1D("matrix_occ")->GetXaxis()->SetTitle("Matrix position");
  getStatistics().getHisto1D("matrix_occ")->GetYaxis()->SetTitle("Number of SigCh");

  // Scins
  getStatistics().createHistogram(
    new TH1F("scin_occ", "Scins occupation", 16, 199.5, 215.5)
  );
  getStatistics().getHisto1D("scin_occ")->GetXaxis()->SetTitle("SCIN ID");
  getStatistics().getHisto1D("scin_occ")->GetYaxis()->SetTitle("Number of SigCh");

  // Histograms per matrix
  for(int scinID=201;scinID<214;scinID++){

    getStatistics().createHistogram(
      new TH1F(
        Form("mtx_%d_A", scinID),
        Form("SiPM occupation in matrix slot %d side A", scinID),
        5, -0.5, 4.5
      )
    );
    getStatistics().getHisto1D(Form("mtx_%d_A", scinID))->GetXaxis()->SetTitle("SiPM matrix position");
    getStatistics().getHisto1D(Form("mtx_%d_A", scinID))->GetYaxis()->SetTitle("Number of SigCh");

    getStatistics().createHistogram(
      new TH1F(
        Form("mtx_%d_B", scinID),
        Form("SiPM occupation in matrix slot %d side B", scinID),
        5, -0.5, 4.5
      )
    );
    getStatistics().getHisto1D(Form("mtx_%d_B", scinID))->GetXaxis()->SetTitle("SiPM matrix position");
    getStatistics().getHisto1D(Form("mtx_%d_B", scinID))->GetYaxis()->SetTitle("Number of SigCh");
  }

  // Flagging histograms
  getStatistics().createHistogram(new TH1F(
    "good_vs_bad_sigch", "Number of good and corrupted SigChs created", 3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "filter_LT_tdiff", "LT time diff", 200, 0.0, 200000.0
  ));
  getStatistics().getHisto1D("filter_LT_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_LT_tdiff")->GetYaxis()->SetTitle("Number of LT pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_LL_PM", "Number of LL found on PMs", 111, 399.5, 510.5
  ));
  getStatistics().getHisto1D("filter_LL_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("filter_LL_PM")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_LL_THR", "Number of found LL on Thresolds", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("filter_LL_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("filter_LL_THR")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_LL_tdiff", "Time diff of LL pairs", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("filter_LL_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_LL_tdiff")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_TT_PM", "Number of TT found on PMs", 111, 399.5, 510.5
  ));
  getStatistics().getHisto1D("filter_TT_PM")->GetXaxis()->SetTitle("PM ID");
  getStatistics().getHisto1D("filter_TT_PM")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_TT_THR", "Number of found TT on Thresolds", 4, 0.5, 4.5
  ));
  getStatistics().getHisto1D("filter_TT_THR")->GetXaxis()->SetTitle("THR Number");
  getStatistics().getHisto1D("filter_TT_THR")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "filter_TT_tdiff", "Time diff of TT pairs", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("filter_TT_tdiff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("filter_TT_tdiff")->GetYaxis()->SetTitle("Number of TT pairs");

  // Time differences of consecutive lead thr1 SigChs after filtering
  getStatistics().createHistogram(new TH1F(
    "consec_lead_THR1", "Time diff of consecutive leadings THR1", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("consec_lead_THR1")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("consec_lead_THR1")->GetYaxis()->SetTitle("Number of SigCh pairs");
}
