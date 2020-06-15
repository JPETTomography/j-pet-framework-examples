/*
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
 *  @file TimeCalibration.cpp
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include "TimeCalibration.h"
#include "TF1.h"
#include "TString.h"
#include <TDirectory.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <JPetOptionsTools/JPetOptionsTools.h>

using namespace jpet_options_tools;
using namespace std;

TimeCalibration::TimeCalibration(const char* name): JPetUserTask(name) {}

TimeCalibration::~TimeCalibration()
{
  delete fBarrelMap;
}

bool TimeCalibration::init()
{
  time_t local_time;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  fBarrelMap = new JPetGeomMapping(getParamBank());

  fOutputEvents = new JPetTimeWindow("JPetEvent");//This line has to be added since starting from v6 we use cointainers
  //for TimeWindows and we need to specify which type of data will be stored
  //in the root tree. For now we do not need to save tree after the calibration
  //so I left the JPetEvent class. For more info ask Alek or Wojtek
//------Calibration run number from options file
  if ( isOptionSet(fParams.getOptions(), fCalibRunKey)) {
    CalibRun = getOptionAsInt(fParams.getOptions(), fCalibRunKey);
  }

  if (isOptionSet(fParams.getOptions(), fTOTcutLow)) {
    TOTcut[0] = getOptionAsFloat(fParams.getOptions(), fTOTcutLow);
  }

  if (isOptionSet(fParams.getOptions(), fTOTcutHigh)) {
    TOTcut[1] = getOptionAsFloat(fParams.getOptions(), fTOTcutHigh);
  }

  if (isOptionSet(fParams.getOptions(), kMainStripKey)) {
    int code = getOptionAsInt(fParams.getOptions(), kMainStripKey);
    LayerToCalib = code / 100; // layer number
    StripToCalib = code % 100; // strip number
  }

  INFO(Form("Calibrating scintillator %d from layer %d.", StripToCalib, LayerToCalib));

  time(&local_time); //get the local time at which we start calibration
  //
  std::ofstream output;

  output.open(OutputFile, std::ios::app); //open the final output file in append mode
  if (output.tellp() == 0) {             //if the file is empty/new write the header
    output << "# Time calibration constants" << std::endl;
    output << "# For side A we apply only the correction from refference detector, for side B the correction is equal to the sum of the A-B" << std::endl;
    output << "# correction and offset with respect to the refference detector. For side A we report the sigmas and chi2/ndf for fit to the time difference spectra with refference detector" << std::endl;
    output << "# while the same quality variables for fits to the A-B time difference are given for B side section" << std::endl;
    output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;
    output << "# Calibration started on " << ctime(&local_time);
  } else {
    output << "# Calibration started on " << ctime(&local_time); //if the file was already on disk write only the time at which the calibration started
    output.close();
  }
  //
  for (int thr = 1; thr <= 4; thr++) { // loop over thresholds

//histos for leading edge
//		  const char * histo_name_l = formatUniqueSlotDescription(scin.at()->getBarrelSlot(), thr, "timeDiffAB_leading_");
//
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_l, histo_name_l, 400, -20., 20.),
                                                            "Time difference AB Leading [ns]", "Counts");
    //
//histograms for leading edge refference detector time difference
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_Ref_l, histo_name_Ref_l, 800, -80., 80.),
                                                            "Time difference AB Leading reference detector [ns]", "Counts");
    //
//histos for trailing edge
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_t, histo_name_t, 400, -20., 20.),
                                                            "Time difference AB Trailing [ns]", "Counts");
    //
//histograms for leading edge refference detector time difference
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_Ref_t, histo_name_Ref_t, 1000, -100., 100.),
                                                            "Time difference AB Trailing reference detector [ns]", "Counts");
    //
  }
  INFO("#############");
  INFO("CALIB_INIT: INITIALIZATION DONE!");
  INFO("#############");

  return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TimeCalibration::exec()
{
  double RefTimeLead[4] = { -1.e43, -1.e43, -1.e43, -1.e43};
  double RefTimeTrail[4] = { -1.e43, -1.e43, -1.e43, -1.e43};
  std::vector <JPetHit> fhitsCalib;
  std::vector <double> fRefTimesL;
  std::vector <double> fRefTimesT;

  const int kPMidRef = 385;
  //getting the data from event in propriate format
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();
    //
    for (uint i = 0; i < n; ++i) {
      const JPetHit& hit = dynamic_cast<const JPetHit&>(timeWindow->operator[](i));
      int PMid = hit.getSignalB().getRecoSignal().getRawSignal().getPM().getID();
      //
      //taking refference detector hits times (scin=193, Pmt=385)
      //

      if (PMid == kPMidRef) {
        auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
        auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
        //
        for (auto& thr_time_pair : lead_times_B) {
          int thr = thr_time_pair.first;
          RefTimeLead[thr] = thr_time_pair.second;

        }

        for (auto& thr_time_pair : trail_times_B) {
          int thr = thr_time_pair.first;
          RefTimeTrail[thr] = thr_time_pair.second;
        }
        fRefTimesL.push_back(RefTimeLead[1]);
        fRefTimesT.push_back(RefTimeTrail[1]);
      } else {
        fhitsCalib.push_back(hit);
      }
    }
    //
    for (auto i = fhitsCalib.begin(); i != fhitsCalib.end(); i++) {
      fillHistosForHit(*i, fRefTimesL, fRefTimesT);
    }
    fhitsCalib.clear();
    fRefTimesL.clear();
    fRefTimesT.clear();

  } else {
    return false;
  }

  return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TimeCalibration::terminate()
{

  //
  //create output txt file with calibration parameters
  //
  std::ofstream results_fit;
  results_fit.open(OutputFile, std::ios::app);
  //
  for (int thr = 1; thr <= 4; thr++) {
//scintillators
//
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", LayerToCalib, StripToCalib, thr);
    //double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
    TH1F* histoToSave_leading = getStatistics().getHisto1D(histo_name_l);
    //
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", LayerToCalib, StripToCalib, thr);
    //double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();

    TH1F* histoToSave_trailing = getStatistics().getHisto1D(histo_name_t);
//reference detector
    //
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", LayerToCalib, StripToCalib, thr);
    //double mean_Ref_l = getStatistics().getHisto1D(histo_name_Ref_l).GetMean();
    TH1F* histoToSave_Ref_leading = getStatistics().getHisto1D(histo_name_Ref_l);
    //
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", LayerToCalib, StripToCalib, thr);
    //double mean_Ref_t = getStatistics().getHisto1D(histo_name_Ref_t).GetMean();
    TH1F* histoToSave_Ref_trailing = getStatistics().getHisto1D(histo_name_Ref_t);
//
    //
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

      //
// writing to apropriate format (txt file)
//We assume that all the corrections will be ADDED to the times of channels
//side A
//C2 = C2 - Cl(warstwa-1) (we correct the correction with respect to ref. detector only for L2 and L3
//offset = -C2 (ref. det) + C1/2 (AB calib)

      float CAl = -(position_peak_Ref_l - Cl[LayerToCalib - 1]) + position_peak_l / 2.;
      float SigCAl = sqrt(pow(position_peak_error_Ref_l / 2., 2) + pow(position_peak_error_l, 2) + pow(SigCl[LayerToCalib - 1], 2));
      float CAt = -(position_peak_Ref_t - Cl[LayerToCalib - 1]) + position_peak_t / 2.;
      float SigCAt = sqrt(pow(position_peak_error_Ref_t / 2., 2) + pow(position_peak_error_t, 2) + pow(SigCl[LayerToCalib - 1], 2));
      //
//side B
//C2 = C2 - Cl(warstwa-1) (we correct the correction with respect to ref. detector only for L2 and L3
//offset = -C2 (ref. det) -C1/2 (AB calib)
      float CBl = -(position_peak_Ref_l - Cl[LayerToCalib - 1]) - position_peak_l / 2.;
      float SigCBl = SigCAl;
      float CBt = -(position_peak_Ref_t - Cl[LayerToCalib - 1]) - position_peak_t / 2.;
      float SigCBt = SigCAt;
      //
      results_fit << LayerToCalib << "\t" << StripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl << "\t" << SigCAl << "\t" << CAt << "\t" << SigCAt << "\t" << sigma_peak_Ref_l
                  << "\t" << sigma_peak_Ref_t << "\t"  << chi2_ndf_Ref_l << "\t" << chi2_ndf_Ref_t << endl;
      //
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

//////////////////////////////////

void TimeCalibration::fillHistosForHit(const JPetHit& hit, const std::vector<double>&   fRefTimesL, const std::vector<double>& fRefTimesT)
{

  auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
//
//-----------TOT calculation for the slot hit
  float TOT_A = 0.;
  float TOT_B = 0.;
  double timeDiffTmin = 0;
  double timeDiffLmin = 0;
  //

  //**	int StripToCalibTemp = hit.getScintillator().getID();
  //**	int LayerToCalibTemp = hit.getBarrelSlot().getLayer().getID();
  //
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
  float tTOT = (TOT_A + TOT_B) / 1000.; //total TOT in ns
//
//----------cut the hit if TOT is out of accepted range (cuts given in ns)
  if (tTOT >= TOTcut[0] && tTOT <= TOTcut[1]) {
//
//leading edge
    for (auto& thr_time_pair : lead_times_A) {

      int thr = thr_time_pair.first;
      //**		const char * histo_name_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_leading_",LayerToCalib,StripToCalib,thr);
      //**const char * histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_leading_",LayerToCalib,StripToCalib,thr);
      //**std::cout << histo_name_l<<" "<<histo_name_l<< std::endl;
      if ( lead_times_B.count(thr) > 0 ) { // if there was leading time at the same threshold at opposite side
        double timeDiffAB_l = lead_times_B[thr] - lead_times_A[thr];
        timeDiffAB_l /= 1000.; // we want the plots in ns instead of ps

        // fill the appropriate histogram
        const char* histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
        //**const char * histo_name_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_leading_",LayerToCalib,StripToCalib,thr);
        getStatistics().fillHistogram(histo_name_l, timeDiffAB_l);
//
//take minimum time difference between Ref and Scint
        timeDiffLmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesL.size(); i++) {
          double timeDiffHit_L = (lead_times_A[thr] + lead_times_B[thr]) / 2. - fRefTimesL[i];
          timeDiffHit_L = timeDiffHit_L / 1000.; //ps -> ns
          if (fabs(timeDiffHit_L) < timeDiffLmin) {
            timeDiffLmin = timeDiffHit_L;
          }
        }
        //**			const char * histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_leading_",LayerToCalib,StripToCalib,thr);
        const char* histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
        if (timeDiffTmin < 100.) {
          getStatistics().fillHistogram(histo_name_Ref_l, timeDiffLmin);
        }
      }
    }



//trailing
    for (auto& thr_time_pair : trail_times_A) {
      int thr = thr_time_pair.first;
      //**const char * histo_name_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_trailing_",LayerToCalib,StripToCalib,thr);
      //**const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_trailing_",LayerToCalib,StripToCalib,thr);

      if ( trail_times_B.count(thr) > 0 ) { // if there was trailing time at the same threshold at opposite side

        double timeDiffAB_t = trail_times_B[thr] - trail_times_A[thr];
        timeDiffAB_t /= 1000.; // we want the plots in ns instead of ps

        //fill the appropriate histogram
        const char* histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
        //**const char * histo_name_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffAB_trailing_",LayerToCalib,StripToCalib,thr);
        getStatistics().fillHistogram(histo_name_t, timeDiffAB_t);
//
//taken minimal time difference between Ref and Scint
        timeDiffTmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesT.size(); i++) {
          double timeDiffHit_T = (trail_times_A[thr] + trail_times_B[thr]) / 2. - fRefTimesT[i];
          timeDiffHit_T = timeDiffHit_T / 1000.; //ps->ns
          if (fabs(timeDiffHit_T) < timeDiffTmin) {
            timeDiffTmin = timeDiffHit_T;
          }
        }
        //**const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d","timeDiffRef_trailing_",LayerToCalib,StripToCalib,thr);
        const char* histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
        if (timeDiffTmin < 100.) {
          getStatistics().fillHistogram(histo_name_Ref_t, timeDiffTmin);
        }
      }
    }
  }//end of the if for TOT cut
}


const char* TimeCalibration::formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix = "")
{

  int slot_number = fBarrelMap->getSlotNumber(slot);
  int layer_number = fBarrelMap->getLayerNumber(slot.getLayer());

  return Form("%slayer_%d_slot_%d_thr_%d", prefix, layer_number, slot_number, threshold);

}
