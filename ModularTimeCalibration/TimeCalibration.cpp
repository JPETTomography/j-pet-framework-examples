/*
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
 *  @file TimeCalibration.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include "TimeCalibration.h"

#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <time.h>

#include <TDirectory.h>
#include <TString.h>
#include <TF1.h>

using namespace jpet_options_tools;
using namespace std;

TimeCalibration::TimeCalibration(const char* name): JPetUserTask(name) {}

TimeCalibration::~TimeCalibration(){}

bool TimeCalibration::init()
{
  time_t local_time;

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Calibration run number from options file
  if (isOptionSet(fParams.getOptions(), kCalibRunKey)) {
    fCalibRun = getOptionAsInt(fParams.getOptions(), kCalibRunKey);
  }

  if (isOptionSet(fParams.getOptions(), kRefDetPMIDKey)) {
    fPMidRef = getOptionAsInt(fParams.getOptions(), kRefDetPMIDKey);
  }

  if (isOptionSet(fParams.getOptions(), kTOTcutLowKey)) {
    fTOTcut[0] = getOptionAsFloat(fParams.getOptions(), kTOTcutLowKey);
  }

  if (isOptionSet(fParams.getOptions(), kTOTcutHighKey)) {
    fTOTcut[1] = getOptionAsFloat(fParams.getOptions(), kTOTcutHighKey);
  }

  if (isOptionSet(fParams.getOptions(), kMainStripKey)) {
    int fStripToCalib = getOptionAsInt(fParams.getOptions(), kMainStripKey);
  }

  INFO(Form("Calibrating scintillator %d.", fStripToCalib));

  // get the local time at which we start calibration
  time(&local_time);
  std::ofstream output;

  //open the final output file in append mode
  output.open(kOutputFile, std::ios::app);

  // TODO prepare printout header for output file
  // if (output.tellp() == 0) {
    //if the file is empty/new write the header
    // output << "# Time calibration constants" << std::endl;
    // output << "# For side A we apply only the correction from refference detector, for side B the correction is equal to the sum of the A-B" << std::endl;
    // output << "# correction and offset with respect to the refference detector. For side A we report the sigmas and chi2/ndf for fit to the time difference spectra with refference detector" << std::endl;
    // output << "# while the same quality variables for fits to the A-B time difference are given for B side section" << std::endl;
    // output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;
    // output << "# Calibration started on " << ctime(&local_time);
  // } else {
    //if the file was already on disk write only the time at which the calibration started
    // output << "# Calibration started on " << ctime(&local_time);
    // output.close();
  // }

  // Calibration histograms - for loop over thresholds
  for (int thr = 1; thr <= fNumberOfThresholds; thr++) {
    // leading edge
    getStatistics().createHistogram(new TH1F(
      Form("ABtDiff_lead_scin_%d_thr_%d", fStripToCalib, thr),
      Form("ABtDiff_lead_scin_%d_thr_%d", fStripToCalib, thr),
      400, -20000.0, 20000.0
    ));
    // leading edge refference detector
    getStatistics().createHistogram(new TH1F(
      Form("ref_ABtDiff_lead_scin_%d_thr_%d", fStripToCalib, thr),
      Form("ref_ABtDiff_lead_scin_%d_thr_%d", fStripToCalib, thr),
      800, -80000.0, 80000.0
    ));
    // trailing edge
    getStatistics().createHistogram(new TH1F(
      Form("ABtDiff_trail_scin_%d_thr_%d", fStripToCalib, thr),
      Form("ABtDiff_trail_scin_%d_thr_%d", fStripToCalib, thr),
      400, -20000.0, 20000.0
    ));
    // histograms for leading edge refference detector time difference
    getStatistics().createHistogram(new TH1F(
      Form("ABtDiff_trail_scin_%d_thr_%d", fStripToCalib, thr),
      Form("ABtDiff_trail_scin_%d_thr_%d", fStripToCalib, thr),
      1000, -100000.0, 100000.0
    ));
  }

  return true;
}

bool TimeCalibration::exec()
{
  double refLeadTHR[fNumberOfThresholds] = { -1.e43 };
  double refTrailTHR[fNumberOfThresholds] = { -1.e43 };

  vector<JPetHit> hitsCalib;
  vector<double> refLeadTimes;
  vector<double> refTrailTimes;

  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {

    for (uint i = 0; i < timeWindow->getNumberOfEvents(); ++i) {

      auto& hit = dynamic_cast<const JPetHit&>(timeWindow->operator[](i));
      int pmID = hit.getSignalB().getPM().getID();

      // taking refference detector hits times (scin=193, Pmt=385)
      if (pmID == fPMidRef) {

        auto lead_times_B = hit.getSignalB().getRawSignals()
        .getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal()
        .getTimesVsThresholdNumber(JPetSigCh::Trailing);

        for (auto& thr_time_pair : lead_times_B) {
          refLeadTHR[thr_time_pair.first] = thr_time_pair.second;
        }

        for (auto& thr_time_pair : trail_times_B) {
          refTrailTHR[thr_time_pair.first] = thr_time_pair.second;
        }

        refLeadTimes.push_back(refLeadTHR[1]);
        refTrailTimes.push_back(refTrailTHR[1]);

      } else {
        hitsCalib.push_back(hit);
      }
    }

    for (auto i = hitsCalib.begin(); i != hitsCalib.end(); i++) {
      fillHistosForHit(*i, refLeadTimes, refTrailTimes);
    }

    hitsCalib.clear();
    refLeadTimes.clear();
    refTrailTimes.clear();

  } else {
    return false;
  }

  return true;
}

bool TimeCalibration::terminate()
{
  // create output txt file with calibration parameters
  std::ofstream results_fit;
  results_fit.open(kOutputFile, std::ios::app);

  for (int thr = 1; thr <= fNumberOfThresholds; thr++) {
    //scintillators
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", LayerToCalib, StripToCalib, thr);
    //double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
    TH1F* histoToSave_leading = getStatistics().getHisto1D(histo_name_l);
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", LayerToCalib, StripToCalib, thr);
    //double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
    TH1F* histoToSave_trailing = getStatistics().getHisto1D(histo_name_t);
    //reference detector
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", LayerToCalib, StripToCalib, thr);
    //double mean_Ref_l = getStatistics().getHisto1D(histo_name_Ref_l).GetMean();
    TH1F* histoToSave_Ref_leading = getStatistics().getHisto1D(histo_name_Ref_l);
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", LayerToCalib, StripToCalib, thr);
    //double mean_Ref_t = getStatistics().getHisto1D(histo_name_Ref_t).GetMean();
    TH1F* histoToSave_Ref_trailing = getStatistics().getHisto1D(histo_name_Ref_t);

    if (histoToSave_leading->GetEntries() != 0 && histoToSave_trailing->GetEntries() != 0
    && histoToSave_Ref_leading->GetEntries() != 0 && histoToSave_Ref_trailing->GetEntries() != 0) {
      INFO("#############");
      INFO("CALIB_INFO: Fitting histogams for layer= " + std::to_string(LayerToCalib) + ", slot= " + std::to_string(StripToCalib) + ", threshold= " + std::to_string(thr));
      INFO("#############");
      if (histoToSave_Ref_leading->GetEntries() <= min_ev) {
        results_fit << "#WARNING: Statistics used to determine the leading edge calibration constant with respect to the refference detector was less than " << min_ev << " events!" << endl;
        WARNING(": Statistics used to determine the leading edge calibration constant with respect to the refference detector was less than " + std::to_string(min_ev) + " events!");
      }
      if (histoToSave_Ref_trailing->GetEntries() <= min_ev) {
        results_fit << "#WARNING: Statistics used to determine the trailing edge calibration constant with respect to the refference detector was less than " << min_ev << " events!" << endl;
        WARNING(": Statistics used to determine the trailing edge calibration constant with respect to the refference detector was less than " + std::to_string(min_ev) + " events!");
      }
      if (histoToSave_leading->GetEntries() <= min_ev) {
        results_fit << "#WARNING: Statistics used to determine the leading edge A-B calibration constant was less than " << min_ev << " events!" << endl;
        WARNING(": Statistics used to determine the leading edge A-B calibration constant was less than" + std::to_string(min_ev) + " events!");
      }
      if (histoToSave_trailing->GetEntries() <= min_ev) {
        results_fit << "#WARNING: Statistics used to determine the trailing edge A-B calibration constant was less than " << min_ev << " events!" << endl;
        WARNING(": Statistics used to determine the trailing edge A-B calibration constant was less than" + std::to_string(min_ev) + " events!");
      }

      //fit scintilators
      int highestBin_l = histoToSave_leading->GetBinCenter(histoToSave_leading->GetMaximumBin());
      histoToSave_leading->Fit("gaus", "", "", highestBin_l - 5, highestBin_l + 5);
      histoToSave_leading->Draw();

      int highestBin_t = histoToSave_trailing->GetBinCenter(histoToSave_trailing->GetMaximumBin());
      histoToSave_trailing->Fit("gaus", "", "", highestBin_t - 5, highestBin_t + 5);
      histoToSave_trailing->Draw();

      TF1* fit_l = histoToSave_leading->GetFunction("gaus");
      TF1* fit_t = histoToSave_trailing->GetFunction("gaus");

      double position_peak_l = fit_l->GetParameter(1);
      double position_peak_error_l = fit_l->GetParError(1);
      double sigma_peak_l = fit_l->GetParameter(2);
      double chi2_ndf_l = fit_l->GetChisquare() / fit_l->GetNDF();

      double position_peak_t = fit_t->GetParameter(1);
      double position_peak_error_t = fit_t->GetParError(1);
      double sigma_peak_t = fit_t->GetParameter(2);
      double chi2_ndf_t = fit_t->GetChisquare() / fit_t->GetNDF();

      if ((position_peak_error_l / position_peak_l) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
      }

      if ((position_peak_error_t / position_peak_t) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
      }

      //fit reference detector
      int highestBin_Ref_l = histoToSave_Ref_leading->GetBinCenter(histoToSave_Ref_leading->GetMaximumBin());
      histoToSave_Ref_leading->Fit("gaus", "", "", highestBin_Ref_l - 5, highestBin_Ref_l + 5); //range for fit
      TF1* fit_Ref_l = histoToSave_Ref_leading->GetFunction("gaus");
      fit_Ref_l->SetRange(highestBin_Ref_l - 20, highestBin_Ref_l + 20); //range to draw gaus function
      histoToSave_Ref_leading->Draw();

      int highestBin_Ref_t = histoToSave_Ref_trailing->GetBinCenter(histoToSave_Ref_trailing->GetMaximumBin());
      histoToSave_Ref_trailing->Fit("gaus", "", "", highestBin_Ref_t - 5, highestBin_Ref_t + 5); //range for fit
      TF1* fit_Ref_t = histoToSave_Ref_trailing->GetFunction("gaus");
      fit_Ref_t->SetRange(highestBin_Ref_t - 20, highestBin_Ref_t + 20); //range to draw gaus function
      histoToSave_Ref_trailing->Draw();

      double position_peak_Ref_l = fit_Ref_l->GetParameter(1);
      double position_peak_error_Ref_l = fit_Ref_l->GetParError(1);
      double sigma_peak_Ref_l = fit_Ref_l->GetParameter(2);
      double chi2_ndf_Ref_l = fit_Ref_l->GetChisquare() / fit_Ref_l->GetNDF();

      double position_peak_Ref_t = fit_Ref_t->GetParameter(1);
      double position_peak_error_Ref_t = fit_Ref_t->GetParError(1);
      double sigma_peak_Ref_t = fit_Ref_t->GetParameter(2);
      double chi2_ndf_Ref_t = fit_Ref_t->GetChisquare() / fit_Ref_t->GetNDF();

      if ((position_peak_error_Ref_l / position_peak_Ref_l) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
      }

      if ((position_peak_error_Ref_t / position_peak_Ref_t) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
      }

      // writing to apropriate format (txt file)
      //We assume that all the corrections will be ADDED to the times of channels
      //side A
      //C2 = C2 - Cl(warstwa-1) (we correct the correction with respect to ref. detector only for L2 and L3
      //offset = -C2 (ref. det) + C1/2 (AB calib)

      float CAl = -(position_peak_Ref_l - Cl[LayerToCalib - 1]) + position_peak_l / 2.;
      float SigCAl = sqrt(pow(position_peak_error_Ref_l / 2., 2) + pow(position_peak_error_l, 2) + pow(SigCl[LayerToCalib - 1], 2));
      float CAt = -(position_peak_Ref_t - Cl[LayerToCalib - 1]) + position_peak_t / 2.;
      float SigCAt = sqrt(pow(position_peak_error_Ref_t / 2., 2) + pow(position_peak_error_t, 2) + pow(SigCl[LayerToCalib - 1], 2));

      //side B
      //C2 = C2 - Cl(warstwa-1) (we correct the correction with respect to ref. detector only for L2 and L3
      //offset = -C2 (ref. det) -C1/2 (AB calib)
      float CBl = -(position_peak_Ref_l - Cl[LayerToCalib - 1]) - position_peak_l / 2.;
      float SigCBl = SigCAl;
      float CBt = -(position_peak_Ref_t - Cl[LayerToCalib - 1]) - position_peak_t / 2.;
      float SigCBt = SigCAt;

      results_fit << LayerToCalib << "\t" << StripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl << "\t" << SigCAl << "\t" << CAt << "\t" << SigCAt << "\t" << sigma_peak_Ref_l
                  << "\t" << sigma_peak_Ref_t << "\t"  << chi2_ndf_Ref_l << "\t" << chi2_ndf_Ref_t << endl;

      results_fit << LayerToCalib << "\t" << StripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl << "\t" << SigCBl << "\t" << CBt << "\t" << SigCBt << "\t" << sigma_peak_l
                  << "\t" << sigma_peak_t << "\t" << chi2_ndf_l << "\t" << chi2_ndf_t << endl;
    } else {
      ERROR(": ONE OF THE HISTOGRAMS FOR THRESHOLD " + std::to_string(thr) + " LAYER " + std::to_string(LayerToCalib) + " SLOT " + std::to_string(StripToCalib) +
            " IS EMPTY, WE CANNOT CALIBRATE IT");
    }
  }
  results_fit.close();
  return true;
}

void TimeCalibration::fillHistosForHit(
  const JPetHit& hit, const vector<double>& fRefTimesL, const vector<double>& fRefTimesT
) {
  auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal()
  .getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal()
  .getTimesVsThresholdNumber(JPetSigCh::Trailing);

  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal()
  .getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal()
  .getTimesVsThresholdNumber(JPetSigCh::Trailing);

  // TOT calculation for the slot hit
  float TOT_A = 0.;
  float TOT_B = 0.;
  double timeDiffTmin = 0;
  double timeDiffLmin = 0;

  //  int StripToCalibTemp = hit.getScintillator().getID();
  //  int LayerToCalibTemp = hit.getBarrelSlot().getLayer().getID();
  for (auto& thr_time_pair : lead_times_A) {
    int thr = thr_time_pair.first;
    if (trail_times_A.count(thr) > 0 ) {
      TOT_A = TOT_A + trail_times_A[thr] - lead_times_A[thr];
    }
  }
  for (auto& thr_time_pair : lead_times_B) {
    int thr = thr_time_pair.first;
    if ( trail_times_B.count(thr) > 0 ) {
      TOT_B = TOT_B + trail_times_B[thr] - lead_times_B[thr];
    }
  }
  // total TOT in ns
  float tTOT = (TOT_A + TOT_B) / 1000.;
  // cut the hit if TOT is out of accepted range (cuts given in ns)
  if (tTOT >= TOTcut[0] && tTOT <= TOTcut[1]) {
    //leading edge
    for (auto& thr_time_pair : lead_times_A) {
      int thr = thr_time_pair.first;
      // const char * histo_name_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_leading_",LayerToCalib,StripToCalib,thr);
      // const char * histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_leading_",LayerToCalib,StripToCalib,thr);
      // std::cout << histo_name_l<<" "<<histo_name_l<< std::endl;

      // if there was leading time at the same threshold at opposite side
      if ( lead_times_B.count(thr) > 0 ) {
        double timeDiffAB_l = lead_times_B[thr] - lead_times_A[thr];
        timeDiffAB_l /= 1000.; // we want the plots in ns instead of ps

        // fill the appropriate histogram
        const char* histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
        // const char * histo_name_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_leading_",LayerToCalib,StripToCalib,thr);
        getStatistics().getHisto1D(histo_name_l)->Fill( timeDiffAB_l);
        //take minimum time difference between Ref and Scint
        timeDiffLmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesL.size(); i++) {
          double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr]) / 2. - fRefTimesL[i];
          timeDiffHit_L = timeDiffHit_L / 1000.; //ps -> ns
          if (fabs(timeDiffHit_L) < timeDiffLmin) {
            timeDiffLmin = timeDiffHit_L;
          }
        }
        // const char * histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_leading_",LayerToCalib,StripToCalib,thr);
        const char* histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
        if (timeDiffTmin < 100.) {
          getStatistics().getHisto1D(histo_name_Ref_l)->Fill(timeDiffLmin);
        }
      }
    }
    //trailing
    for (auto& thr_time_pair : trail_times_A) {
      int thr = thr_time_pair.first;
      // const char * histo_name_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_trailing_",LayerToCalib,StripToCalib,thr);
      // const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_trailing_",LayerToCalib,StripToCalib,thr);

      // if there was trailing time at the same threshold at opposite side
      if (trail_times_B.count(thr) > 0) {

        double timeDiffAB_t = trail_times_B[thr] - trail_times_A[thr];
        // we want the plots in ns instead of ps
        timeDiffAB_t /= 1000.;

        //fill the appropriate histogram
        const char* histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
        // const char * histo_name_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_trailing_",LayerToCalib,StripToCalib,thr);
        getStatistics().getHisto1D(histo_name_t)->Fill( timeDiffAB_t);
        //taken minimal time difference between Ref and Scint
        timeDiffTmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesT.size(); i++) {
          double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr]) / 2. - fRefTimesT[i];
          timeDiffHit_T = timeDiffHit_T / 1000.; //ps->ns
          if (fabs(timeDiffHit_T) < timeDiffTmin) {
            timeDiffTmin = timeDiffHit_T;
          }
        }
        // const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_trailing_",LayerToCalib,StripToCalib,thr);
        const char* histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
        if (timeDiffTmin < 100.) {
          getStatistics().getHisto1D(histo_name_Ref_t)->Fill(timeDiffTmin);
        }
      }
    }
  }
  // end of the if for TOT cut
}
