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
 *  @file CalibrationUnit.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"
#include "../LargeBarrelAnalysis/HitFinderTools.h"
#include <JPetWriter/JPetWriter.h>
#include "CalibrationUnit.h"
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

CalibrationUnit::CalibrationUnit(const char* name): JPetUserTask(name) {}

CalibrationUnit::~CalibrationUnit() {}

bool CalibrationUnit::init()
{
  INFO("Calibration Unit started.");
  
  fSourcePosition.SetXYZ(0.,0.,0.);
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  // Getting TOT calculation type
  if (isOptionSet(fParams.getOptions(), kTOTCalculationType)) {
    fTOTCalculationType = getOptionAsString(fParams.getOptions(), kTOTCalculationType);
  } else {
    WARNING("No TOT calculation option given by the user. Using standard sum.");
  }
  // Parameters for deexcitation TOT cut
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMinParamKey)) {
    fDeexTOTCutMin = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMinParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMinParamKey.c_str(), fDeexTOTCutMin
    ));
  }
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMaxParamKey)) {
    fDeexTOTCutMax = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMaxParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMaxParamKey.c_str(), fDeexTOTCutMax
    ));
  }
  // Parameters for annihilation TOT cut
  if (isOptionSet(fParams.getOptions(), kAnniTOTCutMinParamKey)) {
    fAnniTOTCutMin = getOptionAsFloat(fParams.getOptions(), kAnniTOTCutMinParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kAnniTOTCutMinParamKey.c_str(), fAnniTOTCutMin
    ));
  }
  if (isOptionSet(fParams.getOptions(), kAnniTOTCutMaxParamKey)) {
    fAnniTOTCutMax = getOptionAsFloat(fParams.getOptions(), kAnniTOTCutMaxParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kAnniTOTCutMaxParamKey.c_str(), fAnniTOTCutMax
    ));
  }
  // Getting Z position cut
  if (isOptionSet(fParams.getOptions(), kZpositionCut)) {
    fZpositionCut = getOptionAsFloat(fParams.getOptions(), kZpositionCut);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kZpositionCut.c_str(), fZpositionCut
    ));
  }
  // Getting Effective Length
  if (isOptionSet(fParams.getOptions(), kEffectiveLength)) {
    fEffectiveLength = getOptionAsFloat(fParams.getOptions(), kEffectiveLength);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kEffectiveLength.c_str(), fEffectiveLength
    ));
  }
  // Getting assumed source position
  if (isOptionSet(fParams.getOptions(), kSourcePosition)) {
    std::vector<double> sourcePositionFromTheFile = getOptionAsVectorOfDoubles(fParams.getOptions(), kSourcePosition);
    if (sourcePositionFromTheFile.size() == 3) {
      fSourcePosition.SetXYZ(sourcePositionFromTheFile[0], sourcePositionFromTheFile[1], sourcePositionFromTheFile[2]);
    } else {
      WARNING(Form(
        "Parameter %s expects three floats. User gave %d. Using default value of (0,0,0).",
        kSourcePosition.c_str(), (int)sourcePositionFromTheFile.size()
      ));
    }
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of (0,0,0).",
      kSourcePosition.c_str()
    ));
  }
  // Getting Files with parameters
  if (isOptionSet(fParams.getOptions(), kABcorrectionFile)) {
    fABcorrectionFile = getOptionAsFloat(fParams.getOptions(), kABcorrectionFile);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user.",
      kABcorrectionFile.c_str()
    ));
  }
  if (isOptionSet(fParams.getOptions(), kPALScorrectionFile)) {
    fPALScorrectionFile = getOptionAsFloat(fParams.getOptions(), kPALScorrectionFile);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user.",
      kPALScorrectionFile.c_str()
    ));
  }
  
  double value1, value2, value3, value4;
  double uncert1, uncert2, uncert3, uncert4;
  std::ifstream TDiffAB_Corr(fABcorrectionFile.c_str());
  if (TDiffAB_Corr.is_open()) {
    while (TDiffAB_Corr >> value1 >> uncert1 >> value2 >> uncert2 >> value3 >> uncert3 >> value4 >> uncert4) {
      TimeShiftAB_thr1.push_back(value1);
      TimeShiftAB_thr2.push_back(value2);
      TimeShiftAB_thr3.push_back(value3);
      TimeShiftAB_thr4.push_back(value4);
    }
  } else {
      WARNING(Form(
        "No file - %s - with corrections for calibration of a single module",
        fABcorrectionFile.c_str()
      ));
  }
  TDiffAB_Corr.close();
  
  std::ifstream PALS_Corr(fPALScorrectionFile.c_str());
  if (PALS_Corr.is_open()) {
    while (PALS_Corr >> value1 >> uncert1 >> value2 >> uncert2 >> value3 >> uncert3 >> value4 >> uncert4) {
      PALSTDiff_thr1.push_back(value1);
      PALSTDiff_thr2.push_back(value2);
      PALSTDiff_thr3.push_back(value3);
      PALSTDiff_thr4.push_back(value4);
    }
  } else {
      WARNING(Form(
        "No file - %s - with corrections for calibration between modules",
        fPALScorrectionFile.c_str()
      ));
  }
  PALS_Corr.close();
  
  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  // Initialise hisotgrams
  if(fSaveControlHistos) initialiseHistograms();
  return true;
}

bool CalibrationUnit::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      JPetEvent newEvent = event;
      vector<JPetHit> AnnihilationCandidates, DeexcitationCandidates;
      for (auto hit : event.getHits()) {
        int ScintID = hit.getScintillator().getID();
        double TOT = HitFinderTools::calculateTOT( hit );
        if (fSaveControlHistos) {
          getStatistics().fillHistogram("All_XYpos", hit.getPosX(), hit.getPosY());
          getStatistics().fillHistogram("TOT_vs_ID", TOT, ScintID);
          getStatistics().fillHistogram("Z_vs_ID", hit.getPosZ(), ScintID);
          CalcTDiffAB_afterCalibration(hit);
        }
        if (fAnniTOTCutMin < TOT && TOT < fAnniTOTCutMax && fabs( hit.getPosZ() ) < fZpositionCut) {
          AnnihilationCandidates.push_back(hit);
        } else if (fDeexTOTCutMin < TOT && TOT < fDeexTOTCutMax && fabs( hit.getPosZ() ) < fZpositionCut) {
          DeexcitationCandidates.push_back(hit);
        }					
      }

      if (fSaveControlHistos) {
        if (AnnihilationCandidates.size() == 1 && DeexcitationCandidates.size() == 1) {
          PlotLifetimesForThresholds(AnnihilationCandidates[0], DeexcitationCandidates[0]);
        } else if (AnnihilationCandidates.size() == 2) {
          TVector3 RecoPosition = EventCategorizerTools::calculateAnnihilationPoint(AnnihilationCandidates[0], AnnihilationCandidates[1]);
          getStatistics().fillHistogram("XY_2Annihilation", RecoPosition(1), RecoPosition(0));
          getStatistics().fillHistogram("Z_2Annihilation", RecoPosition(2));
        }
      }
      
      events.push_back(newEvent);
    }
    saveEvents(events);
  } else { return false; }
  return true;
}

bool CalibrationUnit::terminate()
{
  INFO("Calibration iteration completed.");
  return true;
}

void CalibrationUnit::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) { fOutputEvents->add<JPetEvent>(event); }
}

void CalibrationUnit::initialiseHistograms(){

  // General histograms
  getStatistics().createHistogramWithAxes(
    new TH2D("All_XYpos", "Hit position XY", 240, -60.25, 59.75, 240, -60.25, 59.75),
    "Hit X position [cm]", "Hit Y position [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TOT_vs_ID_matched", "TOT of the hit vs scintillator ID", 1200, -19.5, 100.5, 200, 0 , 200),
    "TOT of the hit [ns]", "Scintillator ID"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Z_vs_ID", "Z position of the hit vs scintillator ID", 1000, -49.55, 50.45, 200, 0 , 200),
    "Z position [cm]", "Scintillator ID"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr1_calibrated", "Time difference AB vs scintillator ID for threshold 1 after calibration", 
                                            500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 1 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr2_calibrated", "Time difference AB vs scintillator ID for threshold 2 after calibration", 
                                            500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 2 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr3_calibrated", "Time difference AB vs scintillator ID for threshold 3 after calibration", 
                                            500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 3 [ns]", "ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("TDiff_AB_vs_ID_thr4_calibrated", "Time difference AB vs scintillator ID for threshold 4 after calibration", 
                                            500, -24.75, 24.25, 200, -0.5 , 199.5),
                                            "Time difference AB for threshold 4 [ns]", "ID of the scintillator"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_AnnihilationID_thr1", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 1", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 1 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_AnnihilationID_thr2", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 2", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 2 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_AnnihilationID_thr3", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 3", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 3 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_AnnihilationID_thr4", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 4", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 4 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_DeexcitationID_thr1", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 1", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 1 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_DeexcitationID_thr2", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 2", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 2 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_DeexcitationID_thr3", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 3", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 3 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("PALS_vs_DeexcitationID_thr4", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 4", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 4 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_AnnihilationID_thr1", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 1 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 1 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_AnnihilationID_thr2", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 2 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 2 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_AnnihilationID_thr3", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 3 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 3 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_AnnihilationID_thr4", "Positron Lifetime spectrum for a given Annihilation Hit ID for threshold 4 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 4 [ns]", "Annihilation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_DeexcitationID_thr1", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 1 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 1 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_DeexcitationID_thr2", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 2 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 2 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_DeexcitationID_thr3", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 3 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 3 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Corrected_PALS_vs_DeexcitationID_thr4", "Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold 4 after correction", 
                                            4000, -199.55, 200.45, 200, -0.5 , 199.5),
                                            "Positron lifetime for threshold 4 [ns]", "Deexcitation Hit ID of the scintillator"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH2D("XY_2Annihilation", "XY reconstructed position for 2 Annihilation Hits", 1000, -49.55, 50.45, 1000, -49.55, 50.45),
                                            "Hit horizontal position [cm] (Y - JPET)", "Hit vertical position [cm] (X -JPET)"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("Z_2Annihilation", "Z reconstructed position for 2 Annihilation Hits", 1000, -49.55, 50.45),
                                            "Hit longitudal position [cm] (Z - JPET)", "Counts"
  );
}

void CalibrationUnit::CalcTDiffAB_afterCalibration(JPetHit Hit)
{
  double TDiff_AB = 0.;
  std::vector<JPetSigCh> sigALead = Hit.getSignalA().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigBLead = Hit.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);

  unsigned ScintID = Hit.getScintillator().getID();
  double ABcorrection = 0.;
  if (sigALead.size() > 0 && sigBLead.size() > 0) {
    for (unsigned i = 0; i < sigALead.size() && i < sigBLead.size(); i++) {
      TDiff_AB = (sigBLead.at(i).getValue() - sigALead.at(i).getValue())/1000;
      ABcorrection = 0.;
      switch(i) {
        case 0:
          if (TimeShiftAB_thr1.size() >= ScintID)
            ABcorrection = TimeShiftAB_thr1[ScintID - 1]/2;
          getStatistics().fillHistogram("TDiff_AB_vs_ID_thr1_calibrated", TDiff_AB - ABcorrection, ScintID);
          break;
        case 1:
          if (TimeShiftAB_thr2.size() >= ScintID)
            ABcorrection = TimeShiftAB_thr2[ScintID - 1]/2;
          getStatistics().fillHistogram("TDiff_AB_vs_ID_thr2_calibrated", TDiff_AB - ABcorrection, ScintID);
          break;						  
        case 2:
          if (TimeShiftAB_thr3.size() >= ScintID)
            ABcorrection = TimeShiftAB_thr3[ScintID - 1]/2;
          getStatistics().fillHistogram("TDiff_AB_vs_ID_thr3_calibrated", TDiff_AB - ABcorrection, ScintID);
          break;						  
        case 3:
          if (TimeShiftAB_thr4.size() >= ScintID)
            ABcorrection = TimeShiftAB_thr4[ScintID - 1]/2;
          getStatistics().fillHistogram("TDiff_AB_vs_ID_thr4_calibrated", TDiff_AB - ABcorrection, ScintID);
          break;						  
      }
    }
  }
}

void CalibrationUnit::PlotLifetimesForThresholds(JPetHit Hit1, JPetHit Hit2)
{
  double TDiff = 0.;
  std::vector<JPetSigCh> sigALeadHit1 = Hit1.getSignalA().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigBLeadHit1 = Hit1.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);

  std::vector<JPetSigCh> sigALeadHit2 = Hit2.getSignalA().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> sigBLeadHit2 = Hit2.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);

  unsigned ScintID_1 = Hit1.getScintillator().getID();
  unsigned ScintID_2 = Hit2.getScintillator().getID();
  double POF1 = (Hit1.getPos() - fSourcePosition).Mag(); // Path of Fly
  double POF2 = (Hit2.getPos() - fSourcePosition).Mag();
  double TOFDiff = (POF1 - POF2)/fSpeedOfLight;
  double PALScorrection = 0., ABcorrection = 0.;

  if (sigALeadHit1.size() > 0 && sigBLeadHit1.size() > 0 && sigALeadHit2.size() > 0 && sigBLeadHit2.size() > 0) {
    for (unsigned i = 0; i < sigALeadHit1.size() && i < sigBLeadHit1.size() && i < sigALeadHit2.size() && i < sigBLeadHit2.size(); i++) {
      TDiff = (sigBLeadHit1.at(i).getValue() + sigALeadHit1.at(i).getValue())/(2*1000) - 
              (sigBLeadHit2.at(i).getValue() + sigALeadHit2.at(i).getValue())/(2*1000);  //TimeHit1 - TimeHit2
      PALScorrection = 0.;
      ABcorrection = 0.;
      switch(i) {
        case 0:
          if (TimeShiftAB_thr1.size() >= ScintID_1 && TimeShiftAB_thr1.size() >= ScintID_2)
            ABcorrection = TimeShiftAB_thr1[ScintID_1 - 1]/4 - TimeShiftAB_thr1[ScintID_2 - 1]/4;

          getStatistics().fillHistogram("PALS_vs_AnnihilationID_thr1", TDiff - TOFDiff - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("PALS_vs_DeexcitationID_thr1", TDiff - TOFDiff - ABcorrection, ScintID_2);

          if (PALSTDiff_thr1.size() >= ScintID_1 && PALSTDiff_thr1.size() >= ScintID_2)
            PALScorrection = (PALSTDiff_thr1[ScintID_1 - 1] - PALSTDiff_thr1[ScintID_2 - 1])/2;
          getStatistics().fillHistogram("Corrected_PALS_vs_AnnihilationID_thr1", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("Corrected_PALS_vs_DeexcitationID_thr1", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_2);
          break;
        case 1:
          if (TimeShiftAB_thr2.size() >= ScintID_1 && TimeShiftAB_thr2.size() >= ScintID_2)
            ABcorrection = TimeShiftAB_thr2[ScintID_1 - 1]/4 - TimeShiftAB_thr2[ScintID_2 - 1]/4;

          getStatistics().fillHistogram("PALS_vs_AnnihilationID_thr2", TDiff - TOFDiff - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("PALS_vs_DeexcitationID_thr2", TDiff - TOFDiff - ABcorrection, ScintID_2);

          if (PALSTDiff_thr2.size() >= ScintID_1 && PALSTDiff_thr2.size() >= ScintID_2)
            PALScorrection = (PALSTDiff_thr2[ScintID_1 - 1] - PALSTDiff_thr2[ScintID_2 - 1])/2;
          getStatistics().fillHistogram("Corrected_PALS_vs_AnnihilationID_thr2", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("Corrected_PALS_vs_DeexcitationID_thr2", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_2);
          break;
        case 2:
          if (TimeShiftAB_thr3.size() >= ScintID_1 && TimeShiftAB_thr3.size() >= ScintID_2)
            ABcorrection = TimeShiftAB_thr3[ScintID_1 - 1]/4 - TimeShiftAB_thr3[ScintID_2 - 1]/4;

          getStatistics().fillHistogram("PALS_vs_AnnihilationID_thr3", TDiff - TOFDiff - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("PALS_vs_DeexcitationID_thr3", TDiff - TOFDiff - ABcorrection, ScintID_2);

          if (PALSTDiff_thr3.size() >= ScintID_1 && PALSTDiff_thr3.size() >= ScintID_2)
            PALScorrection = (PALSTDiff_thr3[ScintID_1 - 1] - PALSTDiff_thr3[ScintID_2 - 1])/2;
          getStatistics().fillHistogram("Corrected_PALS_vs_AnnihilationID_thr3", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("Corrected_PALS_vs_DeexcitationID_thr3", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_2);
          break;
        case 3:
          if (TimeShiftAB_thr4.size() >= ScintID_1 && TimeShiftAB_thr4.size() >= ScintID_2)
            ABcorrection = TimeShiftAB_thr4[ScintID_1 - 1]/4 - TimeShiftAB_thr4[ScintID_2 - 1]/4;

          getStatistics().fillHistogram("PALS_vs_AnnihilationID_thr4", TDiff - TOFDiff - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("PALS_vs_DeexcitationID_thr4", TDiff - TOFDiff - ABcorrection, ScintID_2);

          if (PALSTDiff_thr4.size() >= ScintID_1 && PALSTDiff_thr4.size() >= ScintID_2)
            PALScorrection = (PALSTDiff_thr4[ ScintID_1 - 1 ] - PALSTDiff_thr4[ ScintID_2 - 1 ])/2;
          getStatistics().fillHistogram("Corrected_PALS_vs_AnnihilationID_thr4", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_1);
          getStatistics().fillHistogram("Corrected_PALS_vs_DeexcitationID_thr4", TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_2);
          break;
      }
    }
  }
}
