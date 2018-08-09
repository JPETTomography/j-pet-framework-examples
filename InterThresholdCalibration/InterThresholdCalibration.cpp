/*
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****
 *  @file InterThresholdCalibration.cpp
 */
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cctype>
#include "InterThresholdCalibration.h"
#include <TF1.h>
#include <TString.h>
#include <TDirectory.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTimer/JPetTimer.h>

using namespace jpet_options_tools;
using namespace std;

InterThresholdCalibration::InterThresholdCalibration(const char* name): JPetUserTask(name) {}


InterThresholdCalibration::~InterThresholdCalibration()
{
  delete fBarrelMap;
}

bool InterThresholdCalibration::init()
{
  

  JPetTimer timer;

  string local_time = timer.getAllMeasuredTimes();

////////////////////////

  fBarrelMap = new JPetGeomMapping(getParamBank());

  for( int layer = 0; layer < getParamBank().getLayersSize(); layer++)
  {
    kSl_max.push_back(fBarrelMap->getSlotsCount( getParamBank().getLayer(layer) ) );
  }

  fOutputEvents = new JPetTimeWindow("JPetEvent");

//This line has to be added since starting from v6 we use cointainers
  //for TimeWindows and we need to specify which type of data will be stored
  //in the root tree. For now we do not need to save tree after the calibration
  //so I left the JPetEvent class. For more info ask Alek or Wojtek

//------Calibration run number from options file
  if ( isOptionSet(fParams.getOptions(), fCalibRunKey)) 
    fCalibRun = getOptionAsInt(fParams.getOptions(), fCalibRunKey);
  

  if ( isOptionSet(fParams.getOptions(), fOutputFileKey)) 
    fOutputFile = getOptionAsString(fParams.getOptions(), fOutputFileKey);
  

  if ( isOptionSet(fParams.getOptions(), fFrac_errKey)) 
    fFrac_err = getOptionAsDouble(fParams.getOptions(), fFrac_errKey);

  if ( isOptionSet(fParams.getOptions(), fMin_evKey)) 
    fMin_ev = getOptionAsDouble(fParams.getOptions(), fMin_evKey);

  std::ofstream output;

  output.open(fOutputFile, std::ios::app); //open the final output file in append mode
  if (output.tellp() == 0) {             //if the file is empty/new write the header
    output << "# Threshold time calibration constants" << std::endl;
    output << "# correction and offset with respect to the t1 (for thr a)" << std::endl;
    output << "# time differences for thresholds are following: t2-t1 (1), t3-t1 (2), t4-t1 (3)" << std::endl;
    output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | thr time diffr: 1 (ab), 2 (ac), 3 (ad) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing" << std::endl;
    output << "# Calibration started on " << local_time;
  } else {
    output << "# Calibration started on " << local_time; //if the file was already on disk write only the time at which the calibration started
    output.close();
  }

  //histograms

  for (int lay = 1; lay <= kSl_max.size()+1; lay++) { // loop over layers

    for (int sl = 1; sl <= kSl_max[lay - 1]; sl++) { // loop over slots

      for (int thre = 2; thre <= 4; thre++) { // loop over th diffr times


//leading
//histos for side A
        const char* histo_name_l_A = Form("timeDiffA_leading_layer_%d_slot_%d_thr_1%d", lay, sl, thre);
        getStatistics().createHistogram( new TH1F(histo_name_l_A, histo_name_l_A, 200, -2., 2.));


//histos for side B
        const char* histo_name_l_B = Form("timeDiffB_leading_layer_%d_slot_%d_thr_1%d", lay, sl, thre);
        getStatistics().createHistogram(new TH1F(histo_name_l_B, histo_name_l_B, 300, -3., 3.) );

//trailing
//histos for side A
        const char* histo_name_t_A = Form("timeDiffA_trailing_layer_%d_slot_%d_thr_1%d", lay, sl, thre);
        getStatistics().createHistogram( new TH1F(histo_name_t_A, histo_name_t_A, 200, -2., 2.));


//histos for side B
        const char* histo_name_t_B = Form("timeDiffB_trailing_layer_%d_slot_%d_thr_1%d", lay, sl, thre);
        getStatistics().createHistogram(new TH1F(histo_name_t_B, histo_name_t_B, 300, -3., 3.));

      }
    }
  }


  INFO("#############");
  INFO("CALIB_INIT: INITIALIZATION DONE!");
  INFO("#############");

  return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool InterThresholdCalibration::exec()
{


  std::vector <JPetHit> fhitsCalib;


  //getting the data from event in propriate format
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();

    for (uint i = 0; i < n; ++i) {
      const JPetHit& hit = dynamic_cast<const JPetHit&>(timeWindow->operator[](i));

      fhitsCalib.push_back(hit);

    }



    for (auto i = fhitsCalib.begin(); i != fhitsCalib.end(); i++) {
      fillHistosForHit(*i);
    }

    fhitsCalib.clear();

  }



  else {
    return false;
  }

  return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool InterThresholdCalibration::terminate()
{

  //
  //create output txt file with calibration parameters
  //
  std::ofstream results_fit;
  results_fit.open(fOutputFile, std::ios::app);
  //
  for (int lay = 1; lay <= kSl_max.size()+1; lay++) { // loop over layers

    for (int sl = 1; sl <= kSl_max[lay - 1]; sl++) { // loop over slots

      for (int th = 1; th <= 3; th++) { // loop over th diffr times

//leading
//histos for side A
        const char* histo_name_l_A = Form("timeDiffA_leading_layer_%d_slot_%d_thr_1%d", lay, sl, th + 1);
        TH1F* histoToSave_leading_A = getStatistics().getHisto1D(histo_name_l_A);


//histos for side B
        const char* histo_name_l_B = Form("timeDiffB_leading_layer_%d_slot_%d_thr_1%d", lay, sl, th + 1);
        TH1F* histoToSave_leading_B = getStatistics().getHisto1D(histo_name_l_B);

//trailing
//histos for side A
        const char* histo_name_t_A = Form("timeDiffA_trailing_layer_%d_slot_%d_thr_1%d", lay, sl, th + 1);
        TH1F* histoToSave_trailing_A = getStatistics().getHisto1D(histo_name_t_A);


//histos for side B
        const char* histo_name_t_B = Form("timeDiffB_trailing_layer_%d_slot_%d_thr_1%d", lay, sl, th + 1);
        TH1F* histoToSave_trailing_B = getStatistics().getHisto1D(histo_name_t_B);


//
//minimal criteria for histograms

        if (histoToSave_leading_A->GetEntries() != 0 && histoToSave_leading_B->GetEntries() != 0
            && histoToSave_trailing_A->GetEntries() != 0 && histoToSave_trailing_B->GetEntries() != 0) {
          INFO("#############");
          INFO("CALIB_INFO: Fitting histogams for layer= " + std::to_string(lay) + ", slot= " + std::to_string(sl) + ", time diffr threshold= " + std::to_string(th));
          INFO("#############");

          if (histoToSave_leading_A->GetEntries() <= fMin_ev) {
            results_fit << "#WARNING: Statistics used to determine the leading edge (A) threshold calibration constant was less than " << fMin_ev << " events!" << endl;
            WARNING(": Statistics used to determine the leading edge (A) threshold calibration constant was less than " + std::to_string(fMin_ev) + " events!");
          }

          if (histoToSave_leading_B->GetEntries() <= fMin_ev) {
            results_fit << "#WARNING: Statistics used to determine the leading edge (B) threshold calibration constant was less than " << fMin_ev << " events!" << endl;
            WARNING(": Statistics used to determine the leading edge (B) threshold calibration constant was less than " + std::to_string(fMin_ev) + " events!");
          }

          if (histoToSave_trailing_A->GetEntries() <= fMin_ev) {
            results_fit << "#WARNING: Statistics used to determine the trailing edge (A) threshold calibration constant was less than " << fMin_ev << " events!" << endl;
            WARNING(": Statistics used to determine the trailing edge (A) threshold calibration constant was less than " + std::to_string(fMin_ev) + " events!");
          }

          if (histoToSave_trailing_B->GetEntries() <= fMin_ev) {
            results_fit << "#WARNING: Statistics used to determine the trailing edge (B) threshold calibration constant was less than " << fMin_ev << " events!" << endl;
            WARNING(": Statistics used to determine the trailing edge (B) threshold calibration constant was less than " + std::to_string(fMin_ev) + " events!");
          }


//fit scintilators
          double highestBin_l_A = histoToSave_leading_A->GetBinCenter(histoToSave_leading_A->GetMaximumBin());
          histoToSave_leading_A->Fit("gaus", "", "", highestBin_l_A - 0.2, highestBin_l_A + 0.2);
          histoToSave_leading_A->Draw();

          double highestBin_l_B = histoToSave_leading_B->GetBinCenter(histoToSave_leading_B->GetMaximumBin());
          histoToSave_leading_B->Fit("gaus", "", "", highestBin_l_B - 0.2, highestBin_l_B + 0.2);
          histoToSave_leading_B->Draw();

          double highestBin_t_A = histoToSave_trailing_A->GetBinCenter(histoToSave_trailing_A->GetMaximumBin());
          histoToSave_trailing_A->Fit("gaus", "", "", highestBin_t_A - 0.2, highestBin_t_A + 0.2);
          histoToSave_trailing_A->Draw();

          double highestBin_t_B = histoToSave_trailing_B->GetBinCenter(histoToSave_trailing_B->GetMaximumBin());
          histoToSave_trailing_B->Fit("gaus", "", "", highestBin_t_B - 0.2, highestBin_t_B + 0.2);
          histoToSave_trailing_B->Draw();


          TF1* fit_l_A = histoToSave_leading_A->GetFunction("gaus");
          TF1* fit_l_B = histoToSave_leading_B->GetFunction("gaus");
          TF1* fit_t_A = histoToSave_trailing_A->GetFunction("gaus");
          TF1* fit_t_B = histoToSave_trailing_B->GetFunction("gaus");

          double position_peak_l_A = fit_l_A->GetParameter(1);
          double position_peak_error_l_A = fit_l_A->GetParError(1);
          double sigma_peak_l_A = fit_l_A->GetParameter(2);
          double chi2_ndf_l_A = fit_l_A->GetChisquare() / fit_l_A->GetNDF();

          double position_peak_l_B = fit_l_B->GetParameter(1);
          double position_peak_error_l_B = fit_l_B->GetParError(1);
          double sigma_peak_l_B = fit_l_B->GetParameter(2);
          double chi2_ndf_l_B = fit_l_B->GetChisquare() / fit_l_B->GetNDF();

          double position_peak_t_A = fit_t_A->GetParameter(1);
          double position_peak_error_t_A = fit_t_A->GetParError(1);
          double sigma_peak_t_A = fit_t_A->GetParameter(2);
          double chi2_ndf_t_A = fit_t_A->GetChisquare() / fit_t_A->GetNDF();

          double position_peak_t_B = fit_t_B->GetParameter(1);
          double position_peak_error_t_B = fit_t_B->GetParError(1);
          double sigma_peak_t_B = fit_t_B->GetParameter(2);
          double chi2_ndf_t_B = fit_t_B->GetChisquare() / fit_t_B->GetNDF();


          if ((position_peak_error_l_A / position_peak_l_A) >= fFrac_err) {
            results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
          }

          if ((position_peak_error_l_B / position_peak_l_B) >= fFrac_err) {
            results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
          }

          if ((position_peak_error_t_A / position_peak_t_A) >= fFrac_err) {
            results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
          }

          if ((position_peak_error_t_B / position_peak_t_B) >= fFrac_err) {
            results_fit << "#WFIT: Large uncertainty on the calibration constant!" << endl;
          }


// writing to apropriate format (txt file)

//side A
          results_fit << lay << "\t" << sl << "\t" << "A" << "\t" << th << "\t" << position_peak_l_A << "\t" << position_peak_error_l_A << "\t" << position_peak_t_A << "\t" << position_peak_error_t_A << "\t" << sigma_peak_l_A
                      << "\t" << sigma_peak_t_A << "\t"  << chi2_ndf_l_A << "\t" << chi2_ndf_t_A << endl;

//side B
          results_fit << lay << "\t" << sl << "\t" << "B" << "\t" << th << "\t" << position_peak_l_B << "\t" << position_peak_error_l_B << "\t" << position_peak_t_B << "\t" << position_peak_error_t_B << "\t" << sigma_peak_l_B
                      << "\t" << sigma_peak_t_B << "\t"  << chi2_ndf_l_B << "\t" << chi2_ndf_t_B << endl;

        } else {
          ERROR(": ONE OF THE HISTOGRAMS FOR THRESHOLD " + std::to_string(th) + " LAYER " + std::to_string(lay) + " SLOT " + std::to_string(sl) + " IS EMPTY, WE CANNOT CALIBRATE IT");
        }

      }

    }
  }


  results_fit.close();


  return true;
}

//////////////////////////////////

void InterThresholdCalibration::fillHistosForHit(const JPetHit& hit)
{

  auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);


//take slot number for the hit
  int slot_number = hit.getBarrelSlot().getID();
  int layer_number = hit.getBarrelSlot().getLayer().getID();
  int slot_nr;

  if (layer_number == 1) slot_nr = slot_number;
  if (layer_number == 2) slot_nr = slot_number - 48;
  if (layer_number == 3) slot_nr = slot_number - 96;

  double thr_time_diff_t_A[5], thr_time_diff_A[5];
  double thr_time_diff_t_B[5], thr_time_diff_B[5];
  double lead_times_first_A, lead_times_first_B;
  double trail_times_first_A, trail_times_first_B;


//leading edge

//A
  for (auto & thr_time_pair : lead_times_A) {

    int thr = thr_time_pair.first;

    if (lead_times_A.count(thr) > 0 && trail_times_A.count(thr) > 0 && lead_times_A.size() == 4 && trail_times_A.size() == 4) { //exactly 4 thresholds

      lead_times_first_A = lead_times_A[1];

      if (thr >= 2) {

        thr_time_diff_A[thr] = lead_times_A[thr] / 1000 - lead_times_first_A / 1000;

        char* histo_name_l_A = Form("timeDiffA_leading_layer_%d_slot_%d_thr_1%d", layer_number, slot_nr, thr);
        getStatistics().getHisto1D(histo_name_l_A)->Fill(thr_time_diff_A[thr]);

      }

    }
  }

//B
  for (auto & thr_time_pair : lead_times_B) {

    int thr = thr_time_pair.first;

    if (lead_times_B.count(thr) > 0 && trail_times_B.count(thr) > 0 && lead_times_B.size() == 4 && trail_times_B.size() == 4) {

      lead_times_first_B = lead_times_B[1];

      if (thr >= 2) {

        thr_time_diff_B[thr] = lead_times_B[thr] / 1000 - lead_times_first_B / 1000;

        char* histo_name_l_B = Form("timeDiffB_leading_layer_%d_slot_%d_thr_1%d", layer_number, slot_nr, thr);
        getStatistics().getHisto1D(histo_name_l_B)->Fill(thr_time_diff_B[thr]);

      }

    }
  }


//trailing edge
  for (auto & thr_time_pair : trail_times_A) {

    int thr = thr_time_pair.first;

    if (trail_times_A.count(thr) > 0 && lead_times_A.count(thr) > 0 && lead_times_A.size() == 4 && trail_times_A.size() == 4) {

      trail_times_first_A = trail_times_A[1];


      if (thr >= 2) {

        thr_time_diff_t_A[thr] = trail_times_A[thr] / 1000 - trail_times_first_A / 1000;

        char* histo_name_t_A = Form("timeDiffA_trailing_layer_%d_slot_%d_thr_1%d", layer_number, slot_nr, thr);
        getStatistics().getHisto1D(histo_name_t_A)->Fill(thr_time_diff_t_A[thr]);

      }

    }
  }


  for (auto & thr_time_pair : trail_times_B) {

    int thr = thr_time_pair.first;

    if (trail_times_B.count(thr) > 0 && lead_times_B.count(thr) > 0 && lead_times_B.size() == 4 && trail_times_B.size() == 4) {


      trail_times_first_B = trail_times_B[1];

      if (thr >= 2) {

        thr_time_diff_t_B[thr] = trail_times_B[thr] / 1000 - trail_times_first_B / 1000;

        char* histo_name_t_B = Form("timeDiffB_trailing_layer_%d_slot_%d_thr_1%d", layer_number, slot_nr, thr);
        getStatistics().getHisto1D(histo_name_t_B)->Fill(thr_time_diff_t_B[thr]);

      }

    }
  }


}


const char* InterThresholdCalibration::formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix = "")
{

  int slot_number = fBarrelMap->getSlotNumber(slot);
  int layer_number = fBarrelMap->getLayerNumber(slot.getLayer());

  return Form("%slayer_%d_slot_%d_thr_%d", prefix, layer_number, slot_number, threshold);

}
