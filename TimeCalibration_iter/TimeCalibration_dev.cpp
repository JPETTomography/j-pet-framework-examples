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
#include "TimeCalibration_dev.h"
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
  if (fBarrelMap) {
    delete fBarrelMap;
    fBarrelMap = 0;
  }
}

bool TimeCalibration::init()
{
  time_t local_time;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  //fBarrelMap = new JPetGeomMapping(getParamBank());
 //JPetGeomMapping mapper(fParamManager->getParamBank());
  fOutputEvents = new JPetTimeWindow("JPetEvent");//This line has to be added since starting from v6 we use cointainers
  //for TimeWindows and we need to specify which type of data will be stored
  //in the root tree. For now we do not need to save tree after the calibration
  //so I left the JPetEvent class. For more info ask Alek or Wojtek

  auto opts = fParams.getOptions();

  //
//------Lower TOT cut from config (json) file
  if (isOptionSet(opts, fTOTcutLow)) {
    TOTcut[0] = getOptionAsFloat(opts, fTOTcutLow);
  }
//------ Higher TOT cut from config (json) file
  if (isOptionSet(opts, fTOTcutHigh)) {
    TOTcut[1] = getOptionAsFloat(opts, fTOTcutHigh);
  }

//------ Packed strip and layer number from config (json) file
  if (isOptionSet(opts, kMainStripKey)) {
    int code = getOptionAsInt(opts, kMainStripKey);
    LayerToCalib = code / 100; // layer number
    StripToCalib = code % 100; // strip number
  }

  if (isOptionSet(opts, fConstantsLoadingFlag)) {
    flag_corr = getOptionAsInt(opts, fConstantsLoadingFlag);
  }


//------ Max number of iterations from config (json) file
  if (isOptionSet(opts, fMaxIterationNumber)) {
    NiterMax  = getOptionAsInt(opts, fMaxIterationNumber);
  }

//------ Temporary file name(the same as the name of file for CalibLoader
  if (isOptionSet(opts, fTmpOutFile)) {
    OutputFileTmp = getOptionAsString(opts, fTmpOutFile);
  }
  //
  INFO("#############");
  INFO("CALIB_INIT:CALIBRATION INITIALIZATION IN PROGRESS ");
  INFO("#############");
  INFO("WE ARE GOING TO CALIBRATE SCINTILLATOR " + std::to_string(StripToCalib) + " FROM LAYER " + std::to_string(LayerToCalib));
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
  //Temporary file opening
  //
  std::fstream outputtmp;
  std::string zonk1;
  int LayerToCalibTmp = 0;
  int StripToCalibTmp = 0;
  char SideTmp = 0;
  int thrTmp = 0;
  //
  outputtmp.open(OutputFileTmp, std::ios::in | std::ios::out);
  if (outputtmp.is_open()) { //if the tmp file exists read the content
    //read the whole file to load the latest constants fitted previously. Not smart but works..
    while ( !outputtmp.eof() ) {
      getline(outputtmp, zonk1);
      for (int i = 1; i < 5; i++) {
        //
        outputtmp >> LayerToCalibTmp >> StripToCalibTmp >> SideTmp >> thrTmp >> CAlTmp[i] >> SigCAlTmp[i] >> CAtTmp[i] >> SigCAtTmp[i]
                  >> sigma_peak_Ref_lATmp[i] >> sigma_peak_Ref_tATmp[i] >> chi2_ndf_Ref_lATmp[i] >> chi2_ndf_Ref_tATmp[i] >> Niter >> flag_end;
        //
        INFO(std::to_string(LayerToCalibTmp) + " " + std::to_string(StripToCalibTmp) + " " + SideTmp + " " + std::to_string(thrTmp)
             + " " + std::to_string(CAlTmp[i]) + " " + std::to_string(SigCAlTmp[i]) + " " + std::to_string(CAtTmp[i]) + " " + std::to_string(SigCAtTmp[i])
             + " " + std::to_string(sigma_peak_Ref_lATmp[i]) + " " + std::to_string(sigma_peak_Ref_tATmp[i]) + " " + std::to_string(chi2_ndf_Ref_lATmp[i])
             + " " + std::to_string(chi2_ndf_Ref_tATmp[i]));
        //
        outputtmp >> LayerToCalibTmp >> StripToCalibTmp >> SideTmp >> thrTmp >> CBlTmp[i] >> SigCBlTmp[i] >> CBtTmp[i] >> SigCBtTmp[i]
                  >> sigma_peak_Ref_lBTmp[i] >> sigma_peak_Ref_tBTmp[i] >> chi2_ndf_Ref_lBTmp[i] >> chi2_ndf_Ref_tBTmp[i] >> Niter >> flag_end;
        //
        INFO(std::to_string(LayerToCalibTmp) + " " + std::to_string(StripToCalibTmp) + " " + SideTmp + " " + std::to_string(thrTmp)
             + " " + std::to_string(CBlTmp[i]) + " " + std::to_string(SigCBlTmp[i]) + " " + std::to_string(CBtTmp[i]) + " " + std::to_string(SigCBtTmp[i])
             + " " + std::to_string(sigma_peak_Ref_lBTmp[i]) + " " + std::to_string(sigma_peak_Ref_tBTmp[i]) + " " + std::to_string(chi2_ndf_Ref_lBTmp[i])
             + " " + std::to_string(chi2_ndf_Ref_tBTmp[i]));
      }
    }
    Niter = Niter + 1; //Increment the iteration number if the file was not empty
    INFO("ITERATION NUMBER: " + std::to_string(Niter));
    INFO("Temporary file " + OutputFileTmp + " opened and read at " + ctime(&local_time));
  } else {
    std::ofstream outputtmpnew;
    outputtmp.open(OutputFileTmp);
    outputtmp << "# Calibration started on " << ctime(&local_time);
    INFO("Temporary file " + OutputFileTmp + " opened at first iteration at " + ctime(&local_time));
    Niter = 1;
  }
  //
  if (flag_corr == 1) { //user flag to be used only if we disable the calibration loading module
    //or we work with root files after the calibration loading module, by default flag_corr==1
    //since this is much faster than the full analysis done starting from unpacker stage
    //if You really want to use corrections at that level first uncomment in main.cpp the CalibLoader module
    //and change flag_corr to 0
    for (int i = 1; i < 5; i++) {
      CAtCor[i] = CAtTmp[i];
      CBtCor[i] = CBtTmp[i];
      CAlCor[i] = CAlTmp[i];
      CBlCor[i] = CBlTmp[i];
    }
  }
  //
  for (int thr = 1; thr <= 4; thr++) { // loop over thresholds

//histos for leading edge
//		  const char * histo_name_l = formatUniqueSlotDescription(scin.at()->getBarrelSlot(), thr, "timeDiffAB_leading_");
//
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogram( new TH1F(histo_name_l, histo_name_l, 400, -20., 20.) );
    //
//histograms for leading edge refference detector time difference
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogram( new TH1F(histo_name_Ref_l, histo_name_Ref_l, 800, -80., 80.) );
    //
//histos for trailing edge
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogram( new TH1F(histo_name_t, histo_name_t, 400, -20., 20.) );
    //
//histograms for leading edge refference detector time difference
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", LayerToCalib, StripToCalib, thr);
    getStatistics().createHistogram( new TH1F(histo_name_Ref_t, histo_name_Ref_t, 1000, -100., 100.) );
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
        fRefTimesL.push_back(RefTimeLead[1] / 1000.); //We divide here to change ps to ns to omitt doing it later
        fRefTimesT.push_back(RefTimeTrail[1] / 1000.);
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
  }

  return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TimeCalibration::terminate()
{

// save timeDiffAB mean values for each slot and each threshold in a JPetAuxilliaryData object
// so that they are available to the consecutive modules
//	getAuxilliaryData().createMap("timeDiffAB mean values");
//
//create output txt file with calibration parameters
//
  std::ofstream results_fit;
  std::fstream results_fitTmp;
  results_fit.open(OutputFile, std::ios::app);
  results_fitTmp.open(OutputFileTmp, std::ios::app);//out);
  float CAl[5] = {0., 0., 0., 0., 0.};
  float SigCAl[5] = {0., 0., 0., 0., 0.};
  float CAt[5] =  {0., 0., 0., 0., 0.};
  float SigCAt[5] = {0., 0., 0., 0., 0.};
//side B
  float CBl[5] = {0., 0., 0., 0., 0.};
  float SigCBl[5] = {0., 0., 0., 0., 0.};
  float CBt[5] = {0., 0., 0., 0., 0.};
  float SigCBt[5] = {0., 0., 0., 0., 0.};
  //
  double sigma_peak_l[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_l[5] = {0., 0., 0., 0., 0.};
  double sigma_peak_t[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_t[5] =  {0., 0., 0., 0., 0.};
  //	double position_peak_Ref_l[5] = {0.,0.,0.,0.,0.};
  //double position_peak_error_Ref_l[5] = {0.,0.,0.,0.,0.};
  double sigma_peak_Ref_l[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_l[5] = {0., 0., 0., 0., 0.};
  //
  //double position_peak_Ref_t[5] = {0.,0.,0.,0.,0.};
  //double position_peak_error_Ref_t[5] = {0.,0.,0.,0.,0.};
  double sigma_peak_Ref_t[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_t[5] = {0., 0., 0., 0., 0.};
  //
  for (int thr = 1; thr <= 4; thr++) {
//scintillators
//
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", LayerToCalib, StripToCalib, thr);
    double mean_l = getStatistics().getHisto1D(histo_name_l).GetMean();
    //getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_l, mean_l);
    TH1F* histoToSave_leading = &(getStatistics().getHisto1D(histo_name_l));
    //
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", LayerToCalib, StripToCalib, thr);
    double mean_t = getStatistics().getHisto1D(histo_name_t).GetMean();
    //getAuxilliaryData().setValue("timeDiffAB mean values", histo_name_t, mean_t);

    TH1F* histoToSave_trailing = &(getStatistics().getHisto1D(histo_name_t));
//reference detector
    //
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", LayerToCalib, StripToCalib, thr);
    double mean_Ref_l = getStatistics().getHisto1D(histo_name_Ref_l).GetMean();
    //getAuxilliaryData().setValue("timeDiffRef mean values", histo_name_Ref_l, mean_Ref_l);
    TH1F* histoToSave_Ref_leading = &(getStatistics().getHisto1D(histo_name_Ref_l));
    //
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", LayerToCalib, StripToCalib, thr);
    double mean_Ref_t = getStatistics().getHisto1D(histo_name_Ref_t).GetMean();
    //getAuxilliaryData().setValue("timeDiffref mean values", histo_name_Ref_t, mean_Ref_t);
    TH1F* histoToSave_Ref_trailing = &(getStatistics().getHisto1D(histo_name_Ref_t));
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
      sigma_peak_l[thr] = fit_l->GetParameter(2);
      chi2_ndf_l[thr] = fit_l->GetChisquare() / fit_l->GetNDF();

      double position_peak_t = fit_t->GetParameter(1);
      double position_peak_error_t = fit_t->GetParError(1);
      sigma_peak_t[thr] = fit_t->GetParameter(2);
      chi2_ndf_t[thr] = fit_t->GetChisquare() / fit_t->GetNDF();
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


      double position_peak_Ref_l  = fit_Ref_l->GetParameter(1);
      double position_peak_error_Ref_l = fit_Ref_l->GetParError(1);
      sigma_peak_Ref_l[thr] = fit_Ref_l->GetParameter(2);
      chi2_ndf_Ref_l[thr] = fit_Ref_l->GetChisquare() / fit_Ref_l->GetNDF();
      //
      double position_peak_Ref_t = fit_Ref_t->GetParameter(1);
      double position_peak_error_Ref_t = fit_Ref_t->GetParError(1);
      sigma_peak_Ref_t[thr] = fit_Ref_t->GetParameter(2);
      chi2_ndf_Ref_t[thr] = fit_Ref_t->GetChisquare() / fit_Ref_t->GetNDF();
      //
// writing to apropriate format (txt file)
//We assume that all the corrections will be ADDED to the times of channels
//side A
//offset = -C2 (ref. det) + C1/2 (AB calib)
//
      CAl[thr] = -(position_peak_Ref_l) + position_peak_l / 2.;
      SigCAl[thr] = sqrt(pow(position_peak_error_Ref_l / 2., 2) + pow(position_peak_error_l, 2));
      CAt[thr] = -(position_peak_Ref_t) + position_peak_t / 2.;
      SigCAt[thr] = sqrt(pow(position_peak_error_Ref_t / 2., 2) + pow(position_peak_error_t, 2));
      //
//side B
//offset = -C2 (ref. det) -C1/2 (AB calib)
      CBl[thr] = -(position_peak_Ref_l - Cl[LayerToCalib - 1]) - position_peak_l / 2.;
      SigCBl[thr] = SigCAl[thr];
      CBt[thr] = -(position_peak_Ref_t - Cl[LayerToCalib - 1]) - position_peak_t / 2.;
      SigCBt[thr] = SigCAt[thr];
    } else {
      ERROR(": ONE OF THE HISTOGRAMS FOR THRESHOLD " + std::to_string(thr) + " LAYER " + std::to_string(LayerToCalib) + " SLOT " + std::to_string(StripToCalib) +
            " IS EMPTY, WE CANNOT CALIBRATE IT");
    }
  }
  //
  if (CheckIfExitIter(CAl, SigCAl, CBl, SigCBl, CAt, SigCAt, CBt, SigCBt, Niter, NiterMax)) {
    //if the difference between old and new constant are smaller than their uncertanities we end iterations and calibration
    //and correct the final results for different layers (layers synchronization)
    //C2 = C2 - Cl(warstwa-1) (we correct the correction with respect to ref. detector only for L2 and L3
    //Moreover, new offsets need to be added to the ones determined in the previous iteration
    for (int thr = 1; thr <= 4; thr++) {
      if (SigCAl[thr] / abs(CAl[thr]) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant (Side A, leading edge)!" << endl;
      }
      if (SigCBl[thr] / abs(CBl[thr]) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant (Side B leading edge)!" << endl;
      }
      if (SigCAt[thr] / abs(CAt[thr]) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant (Side A, trailing edge)!" << endl;
      }
      if (SigCBt[thr] / abs(CBt[thr]) >= frac_err) {
        results_fit << "#WFIT: Large uncertainty on the calibration constant (Side B trailing edge)!" << endl;
      }
      CAl[thr] = CAl[thr] + CAlTmp[thr]  - Cl[LayerToCalib - 1];
      CAt[thr] = CAt[thr] + CAtTmp[thr] - Cl[LayerToCalib - 1];
      SigCAl[thr] = sqrt(pow(SigCAl[thr], 2) + pow(SigCl[LayerToCalib - 1], 2) + pow(SigCAlTmp[thr], 2) );
      SigCAt[thr] =  sqrt(pow(SigCAt[thr], 2) + pow(SigCl[LayerToCalib - 1], 2) + pow(SigCAtTmp[thr], 2) );
      CBl[thr] = CBl[thr] + CBlTmp[thr] - Cl[LayerToCalib - 1];
      CBt[thr] = CBt[thr] + CBtTmp[thr] - Cl[LayerToCalib - 1];
      SigCBl[thr] = sqrt(pow(SigCBl[thr], 2) + pow(SigCl[LayerToCalib - 1], 2) + pow(SigCBlTmp[thr], 2) );
      SigCBt[thr] =  sqrt(pow(SigCBt[thr], 2) + pow(SigCl[LayerToCalib - 1], 2) + pow(SigCBtTmp[thr], 2) );
      //

      results_fit << LayerToCalib << "\t" << StripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                  << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                  << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << Niter << endl;
      //
      results_fit << LayerToCalib << "\t" << StripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                  << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                  << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << Niter << endl;
      //
      results_fitTmp << LayerToCalib << "\t" << StripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                     << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                     << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << Niter << flag_end << endl;
      //
      results_fitTmp << LayerToCalib << "\t" << StripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                     << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                     << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << Niter << flag_end << std::endl;
    }
  } else {
    //
    //add the last and new constants and determine the new uncertainty
    //
    for (int thr = 1; thr <= 4; thr++) {
      CAl[thr] = CAl[thr] + CAlTmp[thr];
      CAt[thr] = CAt[thr] + CAtTmp[thr];
      SigCAl[thr] = sqrt(pow(SigCAl[thr], 2) + pow(SigCAlTmp[thr], 2) );
      SigCAt[thr] =  sqrt(pow(SigCAt[thr], 2) + pow(SigCAtTmp[thr], 2) );
      CBl[thr] = CBl[thr] + CBlTmp[thr];
      CBt[thr] = CBt[thr] + CBtTmp[thr];
      SigCBl[thr] = sqrt(pow(SigCBl[thr], 2) + pow(SigCBlTmp[thr], 2) );
      SigCBt[thr] =  sqrt(pow(SigCBt[thr], 2) + pow(SigCBtTmp[thr], 2) );
      //
      results_fitTmp << LayerToCalib << "\t" << StripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                     << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                     << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << Niter << flag_end << endl;
      //
      results_fitTmp << LayerToCalib << "\t" << StripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                     << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                     << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << Niter << flag_end << endl;
    }
  }
  results_fit.close();
  results_fitTmp.close();
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
      if ( lead_times_B.count(thr) > 0 ) { // if there was leading time at the same threshold at opposite side
        double timeDiffAB_l = (lead_times_B[thr] / 1000. + CBlCor[thr]) - (lead_times_A[thr] / 1000. + CAlCor[thr]); // we want the plots in ns instead of ps
        // fill the appropriate histogram
        const char* histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
        getStatistics().getHisto1D(histo_name_l).Fill( timeDiffAB_l);
//
//take minimum time difference between Ref and Scint
        timeDiffLmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesL.size(); i++) {
          double timeDiffHit_L = (lead_times_A[thr] / 1000. + CAlCor[thr]) + (lead_times_B[thr] / 1000. + CBlCor[thr]);
          timeDiffHit_L = timeDiffHit_L / 2. - fRefTimesL[i];
          if (fabs(timeDiffHit_L) < timeDiffLmin) {
            timeDiffLmin = timeDiffHit_L;
          }
        }
        const char* histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
        if (timeDiffTmin < 100.) {
          getStatistics().getHisto1D(histo_name_Ref_l).Fill(timeDiffLmin);
        }
      }
    }



//trailing
    for (auto& thr_time_pair : trail_times_A) {
      int thr = thr_time_pair.first;

      if ( trail_times_B.count(thr) > 0 ) { // if there was trailing time at the same threshold at opposite side

        double timeDiffAB_t = (trail_times_B[thr] / 1000. + CBtCor[thr]) - (trail_times_A[thr] / 1000. + CAtCor[thr]); // we want the plots in ns instead of ps
        //fill the appropriate histogram
        const char* histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
        getStatistics().getHisto1D(histo_name_t).Fill( timeDiffAB_t);
//
//taken minimal time difference between Ref and Scint
        timeDiffTmin = 10000000000000.;
        for (unsigned int i = 0; i < fRefTimesT.size(); i++) {
          double timeDiffHit_T = (trail_times_A[thr] / 1000. + CAtCor[thr]) + (trail_times_B[thr] / 1000. + CBtCor[thr]);
          timeDiffHit_T = timeDiffHit_T / 2. - fRefTimesT[i];
          if (fabs(timeDiffHit_T) < timeDiffTmin) {
            timeDiffTmin = timeDiffHit_T;
          }
        }
        const char* histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
        if (timeDiffTmin < 100.) {
          getStatistics().getHisto1D(histo_name_Ref_t).Fill(timeDiffTmin);
        }
      }
    }
  }//end of the if for TOT cut
}


const char* TimeCalibration::formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix = "")
{

  //  assert(fParamManager);
  //JPetGeomMapping mapper(fParamManager->getParamBank());

  // int slot_number =  mapper.getSlotNumber(slot);
  //int layer_number = mapper.getLayerNumber(slot.getLayer());
  //
  //Stupid way to determine the slot and layer number
  //since slot.getID(slot) returns slot number 1-192
  int layer_number = slot.getLayer().getID();
  int slot_number =  slot.getID();
  slot_number = slot_number - (layer_number - 1) * 48;

  return Form("%slayer_%d_slot_%d_thr_%d", prefix, layer_number, slot_number, threshold);

}
//
bool TimeCalibration::CheckIfExitIter(float CAl[], float  SigCAl[], float CBl[], float  SigCBl[], float CAt[], float SigCAt[], float CBt[], float SigCBt[], int Niter, int NiterM )
{
  int exit_flag = 0.;
  bool zonk = false;
  //
  //Function checking if we should finish the iterative calibration. Returns true if the maximum iteration number is reached (given by user)
  // For now we require that the new correction is less then its sigma for ALL trhresholds and edges
  //
  if (Niter == NiterM) {
    zonk = true;
  } else {
    for (int thr = 1; thr <= 4; thr++) {
      if ( abs(CAl[thr]) < SigCAl[thr] && abs(CBl[thr]) < SigCBl[thr] && abs(CAt[thr]) < SigCAt[thr] && abs(CBt[thr]) < SigCBt[thr]) {
        exit_flag++;
      }
    }
    if (exit_flag == 4) {
      zonk = true;
    }
  }
  return zonk;
}
