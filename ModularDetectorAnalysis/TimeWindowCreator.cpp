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
        tdcChannel, channel, fMaxTime, fMinTime,
        getStatistics(), fSaveControlHistos
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
    double lastLLThr1TimeAll = 0.0;
    double lastLLThr1TimeGood = 0.0;
    double lastLLThr2TimeAll = 0.0;
    double lastLLThr2TimeGood = 0.0;
    double lastTTThr1TimeAll = 0.0;
    double lastTTThr1TimeGood = 0.0;
    double lastTTThr2TimeAll = 0.0;
    double lastTTThr2TimeGood = 0.0;

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
            if(sigCh.getChannel().getPM().getScin().getID() >= 201
            && sigCh.getChannel().getPM().getScin().getID() < 214){
              getStatistics().getHisto1D(Form("mtx_%d_A", sigCh.getChannel().getPM().getScin().getID()))
              ->Fill(sigCh.getChannel().getPM().getMatrixPosition());

              if(sigCh.getChannel().getThresholdNumber() == 1 && sigCh.getType() == JPetSigCh::Leading){
                getStatistics().getHisto1D(Form("mtx_%d_A_t1L", sigCh.getChannel().getPM().getScin().getID()))
                ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
              }
            }
          } else if(sigCh.getChannel().getPM().getSide()==JPetPM::SideB){
            getStatistics().getHisto1D("pm_occ_sides")->Fill(3);
            if(sigCh.getChannel().getPM().getScin().getID() >= 201
            && sigCh.getChannel().getPM().getScin().getID() < 214){
              getStatistics().getHisto1D(Form("mtx_%d_B", sigCh.getChannel().getPM().getScin().getID()))
              ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
              if(sigCh.getChannel().getThresholdNumber() == 1 && sigCh.getType() == JPetSigCh::Leading){
                getStatistics().getHisto1D(Form("mtx_%d_B_t1L", sigCh.getChannel().getPM().getScin().getID()))
                ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
              }
            }
          }

          if(sigCh.getType()==JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber()==1){
            getStatistics().getHisto2D("times_t1L_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastLLThr1TimeGood != 0.0){
              getStatistics().getHisto1D("tdiff_LL_THR1_after")->Fill(sigCh.getTime()-lastLLThr1TimeGood);
            }
            lastLLThr1TimeGood = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber()==2){
            getStatistics().getHisto2D("times_t2L_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastLLThr2TimeGood != 0.0){
              getStatistics().getHisto1D("tdiff_LL_THR2_after")->Fill(sigCh.getTime()-lastLLThr2TimeGood);
            }
            lastLLThr2TimeGood = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Trailing && sigCh.getChannel().getThresholdNumber()==1){
            getStatistics().getHisto2D("times_t1T_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastTTThr1TimeGood != 0.0){
              getStatistics().getHisto1D("tdiff_TT_THR1_after")->Fill(sigCh.getTime()-lastTTThr1TimeGood);
            }
            lastTTThr1TimeGood = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Trailing && sigCh.getChannel().getThresholdNumber()==2){
            getStatistics().getHisto2D("times_t2T_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastTTThr2TimeGood != 0.0){
              getStatistics().getHisto1D("tdiff_TT_THR2_after")->Fill(sigCh.getTime()-lastTTThr2TimeGood);
            }
            lastTTThr2TimeGood = sigCh.getTime();
          }

        }

      } else if(sigCh.getRecoFlag()==JPetSigCh::Corrupted){

        if(fSaveControlHistos){

          if(sigCh.getType()==JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber()==1){
            if(lastLLThr1TimeAll != 0.0){
              getStatistics().getHisto1D("tdiff_LL_THR1_before")->Fill(sigCh.getTime()-lastLLThr1TimeAll);
            }
            lastLLThr1TimeAll = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Leading && sigCh.getChannel().getThresholdNumber()==2){
            getStatistics().getHisto2D("times_t2L_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastLLThr2TimeAll != 0.0){
              getStatistics().getHisto1D("tdiff_LL_THR2_before")->Fill(sigCh.getTime()-lastLLThr2TimeAll);
            }
            lastLLThr2TimeAll = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Trailing && sigCh.getChannel().getThresholdNumber()==1){
            getStatistics().getHisto2D("times_t1T_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastTTThr1TimeAll != 0.0){
              getStatistics().getHisto1D("tdiff_TT_THR1_before")->Fill(sigCh.getTime()-lastTTThr1TimeAll);
            }
            lastTTThr1TimeAll = sigCh.getTime();
          }
          if(sigCh.getType()==JPetSigCh::Trailing && sigCh.getChannel().getThresholdNumber()==2){
            getStatistics().getHisto2D("times_t2T_per_pm")
            ->Fill(sigCh.getTime(), sigCh.getChannel().getPM().getID());
            if(lastTTThr2TimeAll != 0.0){
              getStatistics().getHisto1D("tdiff_TT_THR2_before")->Fill(sigCh.getTime()-lastTTThr2TimeAll);
            }
            lastTTThr2TimeAll = sigCh.getTime();
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

    getStatistics().createHistogram(
      new TH1F(
        Form("mtx_%d_A_t1L", scinID),
        Form("SiPM occupation in matrix slot %d side A THR1 Lead", scinID),
        5, -0.5, 4.5
      )
    );
    getStatistics().getHisto1D(Form("mtx_%d_A_t1L", scinID))->GetXaxis()->SetTitle("SiPM matrix position");
    getStatistics().getHisto1D(Form("mtx_%d_A_t1L", scinID))->GetYaxis()->SetTitle("Number of SigCh");

    getStatistics().createHistogram(
      new TH1F(
        Form("mtx_%d_B_t1L", scinID),
        Form("SiPM occupation in matrix slot %d side B THR1 Lead", scinID),
        5, -0.5, 4.5
      )
    );
    getStatistics().getHisto1D(Form("mtx_%d_B_t1L", scinID))->GetXaxis()->SetTitle("SiPM matrix position");
    getStatistics().getHisto1D(Form("mtx_%d_B_t1L", scinID))->GetYaxis()->SetTitle("Number of SigCh");
  }

  getStatistics().createHistogram(
    new TH2F(
      "times_t1L_per_pm",
      "THR1 Lead SigCh times on SiPMs",
      200, fMinTime*1000.0, fMaxTime*1000.0, 111, 399.5, 510.5
    )
  );
  getStatistics().getHisto2D("times_t1L_per_pm")->GetXaxis()->SetTitle("time [ps]");
  getStatistics().getHisto2D("times_t1L_per_pm")->GetYaxis()->SetTitle("SiPM ID");

  getStatistics().createHistogram(
    new TH2F(
      "times_t2L_per_pm",
      "THR2 Lead SigCh times on SiPMs",
      200, fMinTime*1000.0, fMaxTime*1000.0, 111, 399.5, 510.5
    )
  );
  getStatistics().getHisto2D("times_t2L_per_pm")->GetXaxis()->SetTitle("time [ps]");
  getStatistics().getHisto2D("times_t2L_per_pm")->GetYaxis()->SetTitle("SiPM ID");

  getStatistics().createHistogram(
    new TH2F(
      "times_t1T_per_pm",
      "THR1 Trail SigCh times on SiPMs",
      200, fMinTime*1000.0, fMaxTime*1000.0, 111, 399.5, 510.5
    )
  );
  getStatistics().getHisto2D("times_t1T_per_pm")->GetXaxis()->SetTitle("time [ps]");
  getStatistics().getHisto2D("times_t1T_per_pm")->GetYaxis()->SetTitle("SiPM ID");

  getStatistics().createHistogram(
    new TH2F(
      "times_t2T_per_pm",
      "THR2 Trail SigCh times on SiPMs",
      200, fMinTime*1000.0, fMaxTime*1000.0, 111, 399.5, 510.5
    )
  );
  getStatistics().getHisto2D("times_t2T_per_pm")->GetXaxis()->SetTitle("time [ps]");
  getStatistics().getHisto2D("times_t2T_per_pm")->GetYaxis()->SetTitle("SiPM ID");

  // Flagging histograms
  getStatistics().createHistogram(new TH1F(
    "good_vs_bad_sigch", "Number of good and corrupted SigChs created", 3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(1, "GOOD");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(2, "CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetXaxis()->SetBinLabel(3, "UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_sigch")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "LT_time_diff", "LT time diff", 200, 0.0, 100000.0
  ));
  getStatistics().getHisto1D("LT_time_diff")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("LT_time_diff")->GetYaxis()->SetTitle("Number of LL pairs");

  getStatistics().createHistogram(new TH1F(
    "LL_per_PM", "Number of LL found on PMs", 111, 399.5, 510.5
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
    "TT_per_PM", "Number of TT found on PMs", 111, 399.5, 510.5
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

  // Time differences between SigChs before and after flagging
  getStatistics().createHistogram(new TH1F(
    "tdiff_LL_THR1_before", "Time diff of consecutive leadings THR1 before filtering", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("tdiff_LL_THR1_before")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_LL_THR1_before")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_LL_THR1_after", "Time diff of consecutive leadings THR1 after filtering", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("tdiff_LL_THR1_after")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_LL_THR1_after")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_LL_THR2_before", "Time diff of consecutive leadings THR2 before filtering", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("tdiff_LL_THR2_before")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_LL_THR2_before")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_LL_THR2_after", "Time diff of consecutive leadings THR2 after filtering", 200, 0.0, 300000.0
  ));
  getStatistics().getHisto1D("tdiff_LL_THR2_after")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_LL_THR2_after")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_TT_THR1_before", "Time diff of consecutive trailings THR1 before filtering", 200, 0.0, 100000.0
  ));
  getStatistics().getHisto1D("tdiff_TT_THR1_before")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_TT_THR1_before")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_TT_THR1_after", "Time diff of consecutive trailings THR1 after filtering", 200, 0.0, 100000.0
  ));
  getStatistics().getHisto1D("tdiff_TT_THR1_after")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_TT_THR1_after")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_TT_THR2_before", "Time diff of consecutive trailings THR2 before filtering", 200, 0.0, 100000.0
  ));
  getStatistics().getHisto1D("tdiff_TT_THR2_before")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_TT_THR2_before")->GetYaxis()->SetTitle("Number of TT pairs");

  getStatistics().createHistogram(new TH1F(
    "tdiff_TT_THR2_after", "Time diff of consecutive trailings THR2 after filtering", 200, 0.0, 100000.0
  ));
  getStatistics().getHisto1D("tdiff_TT_THR2_after")->GetXaxis()->SetTitle("Time Diff [ps]");
  getStatistics().getHisto1D("tdiff_TT_THR2_after")->GetYaxis()->SetTitle("Number of TT pairs");

}
