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
 *
 *  @file TimeCalibration.cpp
 */

#include "TimeCalibration.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetCommonTools/JPetCommonTools.h>

#include <fstream>
#include <algorithm>

#include <cstdlib>
#include <cstdio>
#include <cctype>

#include <TF1.h>
#include <TString.h>
#include <TDirectory.h>

using namespace jpet_options_tools;
using namespace std;

TimeCalibration::TimeCalibration(const char* name): JPetUserTask(name)
{
}

TimeCalibration::~TimeCalibration()
{
}

bool TimeCalibration::init()
{
  fMapper = jpet_common_tools::make_unique<JPetGeomMapping>(getParamBank());

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (!loadOptions()) {
    ERROR("Error while loading options from configuration file. Check user options!");
    return false;
  }

  INFO("#############");
  INFO("CALIB_INIT:CALIBRATION INITIALIZATION IN PROGRESS ");
  INFO("#############");
  INFO("WE ARE GOING TO CALIBRATE SCINTILLATOR " + std::to_string(fStripToCalib) + " FROM LAYER " + std::to_string(fLayerToCalib));
  fTimer.startMeasurement();
  fTimeConstantsCalibFileNameTmp = std::to_string(fStripToCalib) + "_" + std::to_string(fLayerToCalib)+"_"+fTimeConstantsCalibFileNameTmp;
  loadFileWithParameters(fTimeConstantsCalibFileNameTmp);
  std::ifstream inFile1;
  inFile1.open(fTimeConstantsCalibFileName);
  if (!inFile1) {
    writeHeader(fTimeConstantsCalibFileName);
  }
  else{
      inFile1.close();
      if(Niter==1){
         std::fstream inFile2(fTimeConstantsCalibFileName,std::ios::out | std::ios::app); 
	 inFile2 << "#Calibration started on " << JPetCommonTools::currentDateTime()<<std::endl;
	 inFile2.close();
      }
  }             
  if (fIsCorrection) {
    for (int i = 1; i <= kNumberOfThresholds; i++) {
      CAtCor[i] = CAtTmp[i];
      CBtCor[i] = CBtTmp[i];
      CAlCor[i] = CAlTmp[i];
      CBlCor[i] = CBlTmp[i];
      INFO("Iteration: "+std::to_string(Niter)+", CONSTANTS: "+std::to_string(CAtCor[i])+", "+std::to_string(CBtCor[i])+", "+std::to_string(CAlCor[i])+", "+std::to_string(CBlCor[i]));
    }
  }
  createHistograms();
  INFO("#############");
  INFO("CALIB_INIT: INITIALIZATION DONE!");
  INFO("#############");
  return true;
}

bool TimeCalibration::loadOptions()
{
  auto opts = fParams.getOptions();
  std::vector<std::string> requiredOptions = {kTOTCutLowOptName, kTOTCutHighOptName, kMainStripOptName, kLoadConstantsOptName, kCalibFileTmpOptName, kCalibFileFinalOptName, kPMIdRefOptName,MaxIterNumOptName };

  auto allOptionsExist = std::all_of(requiredOptions.begin(),
                                     requiredOptions.end(),
                                     [&opts](std::string optName)->bool { auto ok = isOptionSet(opts, optName);
                                         if (!ok)
{
  ERROR("No option " + optName + " or bad option format in user param json file.");
  }
  return ok;
                                                                        });
  if (allOptionsExist) {
    TOTcut[0] = getOptionAsFloat(opts, kTOTCutLowOptName);
    TOTcut[1] = getOptionAsFloat(opts, kTOTCutHighOptName);
    int code = getOptionAsInt(opts, kMainStripOptName);
    fLayerToCalib = code / 100;
    fStripToCalib = code % 100;
    fIsCorrection = getOptionAsBool(opts, kLoadConstantsOptName);
    fTimeConstantsCalibFileNameTmp = getOptionAsString(opts, kCalibFileTmpOptName );
    fTimeConstantsCalibFileName = getOptionAsString(opts, kCalibFileFinalOptName );
    kPMIdRef = getOptionAsInt(opts, kPMIdRefOptName);
    NiterMax  = getOptionAsInt(opts,MaxIterNumOptName); 
    return true;
  } else {
    return false;
  }
}

void TimeCalibration::createHistograms()
{
  int NbinABl = 400;
  int NbinReffl = 800;
  int NbinABt = 400;
  int NbinRefft = 1000;
  double tABlmin=0.;
  double tABlmax=0.;
  double tRefflMin=0.;
  double tRefflMax=0;
  double tABtmin=0.;
  double tABtmax=0.;
  double tRefftMin=0.;
  double tRefftMax=0.;
  //
  for (int thr = 1; thr <= kNumberOfThresholds; thr++) {
    assert(fLayerToCalib >= 0);
    assert(fStripToCalib >= 0);
    if(Niter==1)
    {
     tABlmin = -20.;
     tABlmax = 20.;
     tRefflMin = -80.;
     tRefflMax = 80.;
     tABtmin = -20.;
     tABtmax = 20.;
     tRefftMin = -100.;
     tRefftMax = 100;
    }
    else{
      tABlmin = -6.*sigma_peak_lTmp[thr];
      tABlmax =  6.*sigma_peak_lTmp[thr];
      tRefflMin = -6.*sigma_peak_Ref_lTmp[thr];
      tRefflMax = 6.*sigma_peak_Ref_lTmp[thr];
      tABtmin = -6.*sigma_peak_tTmp[thr];
      tABtmax = 6.*sigma_peak_tTmp[thr];
      tRefftMin = -6.*sigma_peak_Ref_tTmp[thr];
      tRefftMax = 6.*sigma_peak_Ref_tTmp[thr];
      NbinReffl = 200;
      NbinRefft = 200;
    }
 
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", fLayerToCalib, fStripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_l, histo_name_l, NbinABl, tABlmin, tABlmax),
                                                            "Time difference AB Leading [ns]", "Counts");
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", fLayerToCalib, fStripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_Ref_l, histo_name_Ref_l, NbinReffl, tRefflMin, tRefflMax),
                                                            "Time difference AB Leading reference detector [ns]", "Counts");
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", fLayerToCalib, fStripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_t, histo_name_t, NbinABt, tABtmin, tABtmax),
                                                            "Time difference AB Trailing [ns]", "Counts");
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", fLayerToCalib, fStripToCalib, thr);
    getStatistics().createHistogramWithAxes( new TH1D(histo_name_Ref_t, histo_name_Ref_t, NbinRefft, tRefftMin, tRefftMax),
                                                            "Time difference AB Trailing reference detector [ns]", "Counts");
  }
}

bool TimeCalibration::exec()
{
  double RefTimeLead[4] = { -1.e43, -1.e43, -1.e43, -1.e43};
  double RefTimeTrail[4] = { -1.e43, -1.e43, -1.e43, -1.e43};
  std::vector <JPetHit> histCalib;
  std::vector <double> refTimesL;
  std::vector <double> refTimesT;

  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    auto n = timeWindow->getNumberOfEvents();
    for (auto i = 0u; i < n; ++i) {
      const JPetHit& hit = dynamic_cast<const JPetHit&>(timeWindow->operator[](i));
      int PMid = hit.getSignalB().getRecoSignal().getRawSignal().getPM().getID();
      if (PMid == kPMIdRef) {
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
        refTimesL.push_back(RefTimeLead[1] / 1000.);
        refTimesT.push_back(RefTimeTrail[1] / 1000.);
      } else {
        if (isInChosenStrip(hit)) {
          histCalib.push_back(hit);
        }
      }
    }
    //
    for (auto i = histCalib.begin(); i != histCalib.end(); i++) {
      fillHistosForHit(*i, refTimesL, refTimesT);
    }
    histCalib.clear();
    refTimesL.clear();
    refTimesT.clear();
  }

  return true;
}

bool TimeCalibration::terminate()
{
  fitAndSaveParametersToFile(fTimeConstantsCalibFileName, fTimeConstantsCalibFileNameTmp);
  if (CheckIfExit(Niter)) {
       auto newOpts = getOptions();
       newOpts["StopIteration_bool"] = true;
       fParams = JPetParams(newOpts, fParams.getParamManagerAsShared());
}
  return true;
}
bool TimeCalibration::CheckIfExit(int Niter)
{
  if(Niter>=NiterMax){
    return true;
  }
  else
    {
      return false;  
    }
}

void TimeCalibration::fillHistosForHit(const JPetHit& hit, const std::vector<double>&   refTimesL, const std::vector<double>& refTimesT)
{
  auto lead_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_A = hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);

  auto lead_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  auto trail_times_B = hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Trailing);
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
  float tTOT = (TOT_A + TOT_B) / 1000.;
  if (tTOT >= TOTcut[0] && tTOT <= TOTcut[1]) {
    for (auto& thr_time_pair : lead_times_A) {

      int thr = thr_time_pair.first;
      if ( lead_times_B.count(thr) > 0 ) {
        double timeDiffAB_l = (lead_times_B[thr] / 1000. + CBlCor[thr]) - (lead_times_A[thr] / 1000. + CAlCor[thr]);
        const char* histo_name_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_leading_");
        getStatistics().fillHistogram(histo_name_l, timeDiffAB_l);
        timeDiffLmin = 10000000000000.;
        for (unsigned int i = 0; i < refTimesL.size(); i++) {
          double timeDiffHit_L = (lead_times_A[thr] / 1000. + CAlCor[thr]) + (lead_times_B[thr] / 1000. + CBlCor[thr]);
          timeDiffHit_L = timeDiffHit_L / 2. - refTimesL[i];
          if (fabs(timeDiffHit_L) < timeDiffLmin) {
            timeDiffLmin = timeDiffHit_L;
          }
        }
        const char* histo_name_Ref_l = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_leading_");
        if (timeDiffTmin < 100.) {
          getStatistics().fillHistogram(histo_name_Ref_l, timeDiffLmin);
        }
      }
    }
    for (auto& thr_time_pair : trail_times_A) {
      int thr = thr_time_pair.first;
      if ( trail_times_B.count(thr) > 0 ) {

        double timeDiffAB_t = (trail_times_B[thr] / 1000. + CBtCor[thr]) - (trail_times_A[thr] / 1000. + CAtCor[thr]);
        const char* histo_name_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffAB_trailing_");
        getStatistics().fillHistogram(histo_name_t, timeDiffAB_t);
        timeDiffTmin = 10000000000000.;
        for (unsigned int i = 0; i < refTimesT.size(); i++) {
          double timeDiffHit_T = (trail_times_A[thr] / 1000. + CAtCor[thr]) + (trail_times_B[thr] / 1000. + CBtCor[thr]);
          timeDiffHit_T = timeDiffHit_T / 2. - refTimesT[i];
          if (fabs(timeDiffHit_T) < timeDiffTmin) {
            timeDiffTmin = timeDiffHit_T;
          }
        }
        const char* histo_name_Ref_t = formatUniqueSlotDescription(hit.getBarrelSlot(), thr, "timeDiffRef_trailing_");
        if (timeDiffTmin < 100.) {
          getStatistics().fillHistogram(histo_name_Ref_t, timeDiffTmin);
        }
      }
    }
  }
}


const char* TimeCalibration::formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix = "")
{
  assert(fMapper);
  int slot_number =  fMapper->getSlotNumber(slot);
  int layer_number = fMapper->getLayerNumber(slot.getLayer());
  return Form("%slayer_%d_slot_%d_thr_%d", prefix, layer_number, slot_number, threshold);
}

bool TimeCalibration::isInChosenStrip(const JPetHit& hit) const
{
  assert(fMapper);
  auto slot = hit.getBarrelSlot();
  int stripNumber =  fMapper->getSlotNumber(slot);
  int layerNumber = fMapper->getLayerNumber(slot.getLayer());
  return (layerNumber == fLayerToCalib) && (stripNumber == fStripToCalib);
}

void TimeCalibration::loadFileWithParameters(const std::string& filename)
{
 
  double chi2_ndf_Ref_lBTmp[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_tBTmp[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_lATmp[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_tATmp[5] = {0., 0., 0., 0., 0.};
  double peak_l[5] = {0., 0., 0., 0., 0.};
  double peak_t[5] = {0., 0., 0., 0., 0.};
  double peak_Ref_l[5] = {0., 0., 0., 0., 0.};
  double peak_Ref_t[5] = {0., 0., 0., 0., 0.};
  //
  double peak_err_l[5] = {0., 0., 0., 0., 0.};
  double peak_err_t[5] = {0., 0., 0., 0., 0.};
  double peak_err_Ref_l[5] = {0., 0., 0., 0., 0.};
  double peak_err_Ref_t[5] = {0., 0., 0., 0., 0.};
  //
  int LayerToCalibTmp = 0;
  int StripToCalibTmp = 0;
  char SideTmpA = 0;
  char SideTmpB = 0;
  int thrTmp = 0;
  std::string line;

  std::ifstream inFile(filename);
  if (inFile) {
    while (getline(inFile, line)) {
      for (int i = 1; i <= kNumberOfThresholds; i++) {
       
        inFile >> LayerToCalibTmp >> StripToCalibTmp >> SideTmpA >> thrTmp >> CAlTmp[i] >> SigCAlTmp[i] >> CAtTmp[i] >> SigCAtTmp[i]
               >> sigma_peak_Ref_lTmp[i] >> sigma_peak_Ref_tTmp[i] >> chi2_ndf_Ref_lATmp[i] >> chi2_ndf_Ref_tATmp[i]>>Niter
	       >>peak_Ref_l[i] >> peak_err_Ref_l[i] >> peak_Ref_t[i] >> peak_err_Ref_t[i];
        inFile >> LayerToCalibTmp >> StripToCalibTmp >> SideTmpB >> thrTmp >> CBlTmp[i] >> SigCBlTmp[i] >> CBtTmp[i] >> SigCBtTmp[i]
               >> sigma_peak_lTmp[i] >> sigma_peak_tTmp[i] >> chi2_ndf_Ref_lBTmp[i] >> chi2_ndf_Ref_tBTmp[i]>>Niter
	       >> peak_l[i] >> peak_err_l[i] >> peak_t[i] >> peak_err_t[i];
      }
    }
      for (int i = 1; i <= kNumberOfThresholds; i++) {          
	 INFO(std::to_string(LayerToCalibTmp) + " " + std::to_string(StripToCalibTmp) + " " + SideTmpA + " " + std::to_string(thrTmp)
             + " " + std::to_string(CAlTmp[i]) + " " + std::to_string(SigCAlTmp[i]) + " " + std::to_string(CAtTmp[i]) + " " + std::to_string(SigCAtTmp[i])
             + " " + std::to_string(sigma_peak_Ref_lTmp[i]) + " " + std::to_string(sigma_peak_Ref_tTmp[i]) + " " + std::to_string(chi2_ndf_Ref_lATmp[i])
             + " " + std::to_string(chi2_ndf_Ref_tATmp[i]));
	 INFO(std::to_string(LayerToCalibTmp) + " " + std::to_string(StripToCalibTmp) + " " + SideTmpB + " " + std::to_string(thrTmp)
             + " " + std::to_string(CBlTmp[i]) + " " + std::to_string(SigCBlTmp[i]) + " " + std::to_string(CBtTmp[i]) + " " + std::to_string(SigCBtTmp[i])
             + " " + std::to_string(sigma_peak_lTmp[i]) + " " + std::to_string(sigma_peak_tTmp[i]) + " " + std::to_string(chi2_ndf_Ref_lBTmp[i])
             + " " + std::to_string(chi2_ndf_Ref_tBTmp[i]));
      }
    Niter = Niter + 1; //Increment the iteration number if the file was on the disk.
    INFO("ITERATION NUMBER: " + std::to_string(Niter));
    INFO("Temporary file " + filename + " opened and read at " + JPetCommonTools::currentDateTime());
  }
  else {
    inFile.close();
    std::ofstream inFile1(filename, std::ios::app);
    inFile1 << "# Calibration started on " << JPetCommonTools::currentDateTime() << std::endl;
    INFO("Temporary file " + filename + " opened at first iteration at " + JPetCommonTools::currentDateTime());
    Niter = 1;//If the tmp file was not present set the iteration number to 1, prevents e.g. removing the tmp file
  }
}

void TimeCalibration::fitAndSaveParametersToFile(const std::string& filename, const std::string& tmpFilename)
{
  int min_ev = 100;     //minimal number of events for a distribution to be fitted
  double frac_err = 0.3; //maximal fractional uncertainty of parameters accepted by calibration

  std::ofstream results_fit(filename, std::ios::app);
  std::ofstream results_fitTmp(tmpFilename, std::ios::app);

//side A
  double CAl[5] = {0., 0., 0., 0., 0.};
  double SigCAl[5] = {0., 0., 0., 0., 0.};
  double CAt[5] =  {0., 0., 0., 0., 0.};
  double SigCAt[5] = {0., 0., 0., 0., 0.};
//side B
  double  CBl[5] = {0., 0., 0., 0., 0.};
  double  SigCBl[5] = {0., 0., 0., 0., 0.};
  double  CBt[5] = {0., 0., 0., 0., 0.};
  double  SigCBt[5] = {0., 0., 0., 0., 0.};

  double sigma_peak_l[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_l[5] = {0., 0., 0., 0., 0.};
  double sigma_peak_t[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_t[5] =  {0., 0., 0., 0., 0.};
  double sigma_peak_Ref_l[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_l[5] = {0., 0., 0., 0., 0.};
  double sigma_peak_Ref_t[5] = {0., 0., 0., 0., 0.};
  double chi2_ndf_Ref_t[5] = {0., 0., 0., 0., 0.};
  double position_peak_l[5] = {0., 0., 0., 0., 0.};
  double position_peak_t[5] = {0., 0., 0., 0., 0.};
  double position_peak_error_l[5] = {0., 0., 0., 0., 0.};
  double position_peak_error_t[5] = {0., 0., 0., 0., 0.};
  double position_peak_Ref_l[5] = {0., 0., 0., 0., 0.};
  double position_peak_Ref_t[5] = {0., 0., 0., 0., 0.};
  double position_peak_error_Ref_l[5] = {0., 0., 0., 0., 0.};
  double position_peak_error_Ref_t[5] = {0., 0., 0., 0., 0.};

  for (int thr = 1; thr <= kNumberOfThresholds; thr++) {
    const char* histo_name_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_leading_", fLayerToCalib, fStripToCalib, thr);
    auto histoToSave_leading = getStatistics().getHisto1D(histo_name_l);
    const char* histo_name_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffAB_trailing_", fLayerToCalib, fStripToCalib, thr);
    auto histoToSave_trailing = getStatistics().getHisto1D(histo_name_t);
    assert(histoToSave_leading);
    assert(histoToSave_trailing);
    const char* histo_name_Ref_l = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_leading_", fLayerToCalib, fStripToCalib, thr);
    auto histoToSave_Ref_leading = getStatistics().getHisto1D(histo_name_Ref_l);
    assert(histoToSave_Ref_leading);
    const char* histo_name_Ref_t = Form("%slayer_%d_slot_%d_thr_%d", "timeDiffRef_trailing_", fLayerToCalib, fStripToCalib, thr);
    auto histoToSave_Ref_trailing = getStatistics().getHisto1D(histo_name_Ref_t);
    assert(histoToSave_Ref_trailing);

    if (histoToSave_leading->GetEntries() != 0 && histoToSave_trailing->GetEntries() != 0
        && histoToSave_Ref_leading->GetEntries() != 0 && histoToSave_Ref_trailing->GetEntries() != 0) {
      INFO("#############");
      INFO("CALIB_INFO: Fitting histogams for layer= " + std::to_string(fLayerToCalib) + ", slot= " + std::to_string(fStripToCalib) + ", threshold= " + std::to_string(thr));
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
      int highestBin_l = histoToSave_leading->GetBinCenter(histoToSave_leading->GetMaximumBin());
      assert(histoToSave_leading->Integral() > 0);
      histoToSave_leading->Fit("gaus", "", "", highestBin_l - 20, highestBin_l + 20);

      int highestBin_t = histoToSave_trailing->GetBinCenter(histoToSave_trailing->GetMaximumBin());
      assert(histoToSave_trailing->Integral() > 0);
      histoToSave_trailing->Fit("gaus", "", "", highestBin_t - 20, highestBin_t + 20);

      TF1* fit_l = histoToSave_leading->GetFunction("gaus");
      assert(fit_l);
      TF1* fit_t = histoToSave_trailing->GetFunction("gaus");
      assert(fit_t);

      position_peak_l[thr] = fit_l->GetParameter(1);
      position_peak_error_l[thr] = fit_l->GetParError(1);
      sigma_peak_l[thr] = fit_l->GetParameter(2);
      chi2_ndf_l[thr] = fit_l->GetChisquare() / fit_l->GetNDF();

      position_peak_t[thr] = fit_t->GetParameter(1);
      position_peak_error_t[thr] = fit_t->GetParError(1);
      sigma_peak_t[thr] = fit_t->GetParameter(2);
      chi2_ndf_t[thr] = fit_t->GetChisquare() / fit_t->GetNDF();

      int highestBin_Ref_l = histoToSave_Ref_leading->GetBinCenter(histoToSave_Ref_leading->GetMaximumBin());
      assert( histoToSave_Ref_leading->Integral() > 0);
      histoToSave_Ref_leading->Fit("gaus", "", "", highestBin_Ref_l - 5, highestBin_Ref_l + 5); //range for fit
      TF1* fit_Ref_l = histoToSave_Ref_leading->GetFunction("gaus");
      assert(fit_Ref_l);
      int highestBin_Ref_t = histoToSave_Ref_trailing->GetBinCenter(histoToSave_Ref_trailing->GetMaximumBin());
      assert( histoToSave_Ref_trailing->Integral() > 0);
      histoToSave_Ref_trailing->Fit("gaus", "", "", highestBin_Ref_t - 5, highestBin_Ref_t + 5); //range for fit
      TF1* fit_Ref_t = histoToSave_Ref_trailing->GetFunction("gaus");
      assert(fit_Ref_t);


      position_peak_Ref_l[thr]  = fit_Ref_l->GetParameter(1);
      position_peak_error_Ref_l[thr] = fit_Ref_l->GetParError(1);
      sigma_peak_Ref_l[thr] = fit_Ref_l->GetParameter(2);
      if (fit_Ref_l->GetNDF() != 0) {
        chi2_ndf_Ref_l[thr] = fit_Ref_l->GetChisquare() / fit_Ref_l->GetNDF();
      } else {
        WARNING("number of degree of freedom from fit is 0");
        chi2_ndf_Ref_l[thr] = -1;
      }

      position_peak_Ref_t[thr] = fit_Ref_t->GetParameter(1);
      position_peak_error_Ref_t[thr] = fit_Ref_t->GetParError(1);
      sigma_peak_Ref_t[thr] = fit_Ref_t->GetParameter(2);
      if (fit_Ref_t->GetNDF() != 0) {
        chi2_ndf_Ref_t[thr] = fit_Ref_t->GetChisquare() / fit_Ref_t->GetNDF();
      } else {
        WARNING("number of degree of freedom from fit is 0");
        chi2_ndf_Ref_t[thr] = -1;
      }

      CAl[thr] = -(position_peak_Ref_l[thr]) + position_peak_l[thr] / 2.;
      SigCAl[thr] = sqrt(pow(position_peak_error_Ref_l[thr] / 2., 2) + pow(position_peak_error_l[thr], 2));
      CAt[thr] = -(position_peak_Ref_t[thr]) + position_peak_t[thr] / 2.;
      SigCAt[thr] = sqrt(pow(position_peak_error_Ref_t[thr] / 2., 2) + pow(position_peak_error_t[thr], 2));
      CBl[thr] = -(position_peak_Ref_l[thr] - Cl[fLayerToCalib - 1]) - position_peak_l[thr] / 2.;
      SigCBl[thr] = SigCAl[thr];
      CBt[thr] = -(position_peak_Ref_t[thr] - Cl[fLayerToCalib - 1]) - position_peak_t[thr] / 2.;
      SigCBt[thr] = SigCAt[thr];
    } else {
      ERROR(": ONE OF THE HISTOGRAMS FOR THRESHOLD " + std::to_string(thr) + " LAYER " + std::to_string(fLayerToCalib) + " SLOT " + std::to_string(fStripToCalib) +
            " IS EMPTY, WE CANNOT CALIBRATE IT");
    }
  }
 if(CheckIfExit(Niter)){
  for (int thr = 1; thr <= kNumberOfThresholds; thr++) {
    CAl[thr] = CAl[thr] + CAlTmp[thr]  - Cl[fLayerToCalib - 1];
    CAt[thr] = CAt[thr] + CAtTmp[thr] - Cl[fLayerToCalib - 1];
    SigCAl[thr] = sqrt(pow(SigCAl[thr], 2) + pow(SigCl[fLayerToCalib - 1], 2) + pow(SigCAlTmp[thr], 2) );
    SigCAt[thr] =  sqrt(pow(SigCAt[thr], 2) + pow(SigCl[fLayerToCalib - 1], 2) + pow(SigCAtTmp[thr], 2) );
    CBl[thr] = CBl[thr] + CBlTmp[thr] - Cl[fLayerToCalib - 1];
    CBt[thr] = CBt[thr] + CBtTmp[thr] - Cl[fLayerToCalib - 1];
    SigCBl[thr] = sqrt(pow(SigCBl[thr], 2) + pow(SigCl[fLayerToCalib - 1], 2) + pow(SigCBlTmp[thr], 2) );
    SigCBt[thr] =  sqrt(pow(SigCBt[thr], 2) + pow(SigCl[fLayerToCalib - 1], 2) + pow(SigCBtTmp[thr], 2) );
    //
    results_fit << fLayerToCalib << "\t" << fStripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << "\t"<< std::endl;
    //
    results_fit << fLayerToCalib << "\t" << fStripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << "\t"<< std::endl;
    
    results_fitTmp << fLayerToCalib << "\t" << fStripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                   << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                   << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << "\t" << Niter <<"\t"
		   << position_peak_Ref_l[thr] << "\t"<< position_peak_error_Ref_l[thr]<<"\t"<<position_peak_Ref_t[thr]<<"\t"<<position_peak_error_Ref_t[thr]
		   <<Niter<<std::endl;
    //
    results_fitTmp << fLayerToCalib << "\t" << fStripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                   << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                   << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << "\t" << Niter <<"\t"
		   << position_peak_l[thr]<<"\t"<< position_peak_error_l[thr]<<"\t" <<position_peak_t[thr]<<"\t"<< position_peak_error_t[thr]
		   << Niter<<std::endl;
  }
 }
 else
   {
  for (int thr = 1; thr <= kNumberOfThresholds; thr++) {
    //
    CAl[thr] = CAl[thr] + CAlTmp[thr];
    CAt[thr] = CAt[thr] + CAtTmp[thr];
    SigCAl[thr] = sqrt(pow(SigCAl[thr], 2) + pow(SigCAlTmp[thr], 2) );
    SigCAt[thr] =  sqrt(pow(SigCAt[thr], 2) + pow(SigCAtTmp[thr], 2) );
    CBl[thr] = CBl[thr] + CBlTmp[thr];
    CBt[thr] = CBt[thr] + CBtTmp[thr];
    SigCBl[thr] = sqrt(pow(SigCBl[thr], 2) + pow(SigCBlTmp[thr], 2) );
    SigCBt[thr] =  sqrt(pow(SigCBt[thr], 2) + pow(SigCBtTmp[thr], 2) );
    //
     assert(abs(CAl[thr]) != 0);
    if (SigCAl[thr] / abs(CAl[thr]) >= frac_err) {
      results_fitTmp << "#WFIT: Large uncertainty on the calibration constant (Side A, leading edge)!" << endl;
    }
    assert(abs(CBl[thr]) != 0);
    if (SigCBl[thr] / abs(CBl[thr]) >= frac_err) {
      results_fitTmp << "#WFIT: Large uncertainty on the calibration constant (Side B leading edge)!" << endl;
    }
    assert(abs(CAt[thr]) != 0);
    if (SigCAt[thr] / abs(CAt[thr]) >= frac_err) {
      results_fitTmp << "#WFIT: Large uncertainty on the calibration constant (Side A, trailing edge)!" << endl;
    }
    assert(abs(CBl[thr]) != 0);
    if (SigCBt[thr] / abs(CBt[thr]) >= frac_err) {
      results_fitTmp << "#WFIT: Large uncertainty on the calibration constant (Side B trailing edge)!" << endl;
    }
    //
    results_fitTmp << fLayerToCalib << "\t" << fStripToCalib << "\t" << "A" << "\t" << thr << "\t" << CAl[thr] << "\t" << SigCAl[thr]
                   << "\t" << CAt[thr] << "\t" << SigCAt[thr] << "\t" << sigma_peak_Ref_l[thr]
                   << "\t" << sigma_peak_Ref_t[thr] << "\t"  << chi2_ndf_Ref_l[thr] << "\t" << chi2_ndf_Ref_t[thr] << "\t" << Niter <<"\t"
                   << position_peak_Ref_l[thr] << "\t"<< position_peak_error_Ref_l[thr]<<"\t"<<position_peak_Ref_t[thr]<<"\t"<<position_peak_error_Ref_t[thr]
		   << std::endl;
    //
    results_fitTmp << fLayerToCalib << "\t" << fStripToCalib << "\t" << "B" << "\t" << thr << "\t" << CBl[thr] << "\t" << SigCBl[thr]
                   << "\t" << CBt[thr] << "\t" << SigCBt[thr] << "\t" << sigma_peak_l[thr]
                   << "\t" << sigma_peak_t[thr] << "\t" << chi2_ndf_l[thr] << "\t" << chi2_ndf_t[thr] << "\t" << Niter <<"\t"
		   << position_peak_l[thr]<<"\t"<< position_peak_error_l[thr]<<"\t" <<position_peak_t[thr]<<"\t"<< position_peak_error_t[thr]
		   <<std::endl;
  }
   }
  results_fit.close();
  results_fitTmp.close();
}

void TimeCalibration::writeHeader(const std::string& filename)
{
  std::ofstream output(filename);
  output << "# Time calibration constants" << std::endl;
  output << "# For side A we apply only the correction from refference detector, for side B the correction is equal to the sum of the A-B" << std::endl;
  output << "# correction and offset with respect to the refference detector. For side A we report the sigmas and chi2/ndf for fit to the time difference spectra with refference detector" << std::endl;
  output << "# while the same quality variables for fits to the A-B time difference are given for B side section" << std::endl;
  output << "# Description of the parameters: layer(1-3) | slot(1-48/96) | side(A-B) | threshold(1-4) | offset_value_leading | offset_uncertainty_leading | offset_value_trailing | offset_uncertainty_trailing | sigma_offset_leading | sigma_offset_trailing | (chi2/ndf)_leading | (chi2/ndf)_trailing|"  << std::endl;
  output << "# Calibration started on " << JPetCommonTools::currentDateTime()<< std::endl;
}
