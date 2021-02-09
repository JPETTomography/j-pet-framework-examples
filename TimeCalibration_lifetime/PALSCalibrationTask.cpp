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
 *  @file PALSCalibrationTask.cpp
 */

#include "../LargeBarrelAnalysis/EventCategorizerTools.h"
#include "../LargeBarrelAnalysis/UniversalFileLoader.h"
#include "../LargeBarrelAnalysis/HitFinderTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetWriter/JPetWriter.h>
#include "PALSCalibrationTask.h"
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

PALSCalibrationTask::PALSCalibrationTask(const char* name): JPetUserTask(name) {}

PALSCalibrationTask::~PALSCalibrationTask() {}

bool PALSCalibrationTask::init()
{
  INFO("PALS Calibration Task started.");
  
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
  //Getting Time window size i norder to properly create PALS histograms
  if (isOptionSet(fParams.getOptions(), kEventTimeWindowKey)) {
    fEventTimeWindow = getOptionAsFloat(fParams.getOptions(), kEventTimeWindowKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kEventTimeWindowKey.c_str(), fEventTimeWindow
    ));
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
    fABcorrectionFile = getOptionAsString(fParams.getOptions(), kABcorrectionFile);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user.",
      kABcorrectionFile.c_str()
    ));
  }
  if (isOptionSet(fParams.getOptions(), kNewVelocitiesFile)) {
    fNewVelocitiesFile = getOptionAsString(fParams.getOptions(), kNewVelocitiesFile);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user.",
      kNewVelocitiesFile.c_str()
    ));
  }
  if (isOptionSet(fParams.getOptions(), kPALScorrectionFile)) {
    fPALScorrectionFile = getOptionAsString(fParams.getOptions(), kPALScorrectionFile);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user.",
      kPALScorrectionFile.c_str()
    ));
  }
  // Number of thresholds for which TDiffAB histograms should be filled
  if (isOptionSet(fParams.getOptions(), kNmbOfThresholdsParamKey)) {
    fNmbOfThresholds = getOptionAsInt(fParams.getOptions(), kNmbOfThresholdsParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %d.",
      kNmbOfThresholdsParamKey.c_str(), fNmbOfThresholds
    ));
  }
  
  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibrationAB = UniversalFileLoader::loadConfigurationParameters(fABcorrectionFile, tombMap);
  fVelocityCalibration = UniversalFileLoader::loadConfigurationParameters(fNewVelocitiesFile, tombMap);
  fTimeCalibrationPALS = UniversalFileLoader::loadConfigurationParameters(fPALScorrectionFile, tombMap);
  
  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  // Initialise hisotgrams
  if(fSaveControlHistos) initialiseHistograms();
  return true;
}

bool PALSCalibrationTask::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
  //  vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
   //   JPetEvent newEvent = event;
      vector<JPetHit> AnnihilationCandidates, DeexcitationCandidates;
      for (auto hit : event.getHits()) {
        int ScintID = hit.getScintillator().getID();
        double TOT = HitFinderTools::calculateTOT(hit);
        if (fSaveControlHistos) {
          getStatistics().fillHistogram("All_XYpos", hit.getPosX(), hit.getPosY());
          getStatistics().fillHistogram("TOT_vs_ID_matched", TOT, ScintID);
          PlotTDiffAB_afterCalibration(hit);
          hit.setPosZ(CorrectZPosition(hit));
          getStatistics().fillHistogram("Z_vs_ID", hit.getPosZ(), ScintID);
        }
        if (fAnniTOTCutMin < TOT && TOT < fAnniTOTCutMax && fabs( hit.getPosZ() ) < fZpositionCut) {
          AnnihilationCandidates.push_back(hit);
        } else if (fDeexTOTCutMin < TOT && TOT < fDeexTOTCutMax && fabs( hit.getPosZ() ) < fZpositionCut) {
          DeexcitationCandidates.push_back(hit);
        }					
      }

      if (fSaveControlHistos) {
        if (AnnihilationCandidates.size() == 1 && DeexcitationCandidates.size() == 1) {
          PlotLifetimesForThresholds(AnnihilationCandidates.at(0), DeexcitationCandidates.at(0));
        } else if (AnnihilationCandidates.size() == 2) {
          TVector3 RecoPosition = EventCategorizerTools::calculateAnnihilationPoint(AnnihilationCandidates.at(0), AnnihilationCandidates.at(1));
          getStatistics().fillHistogram("XY_2Annihilation", RecoPosition(1), RecoPosition(0));
          getStatistics().fillHistogram("Z_2Annihilation", RecoPosition(2));
        }
      }
      
    //  events.push_back(newEvent);
    }
 //   saveEvents(events);
  } else { return false; }
  return true;
}

bool PALSCalibrationTask::terminate()
{
  INFO("Calibration iteration completed.");
  return true;
}

void PALSCalibrationTask::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) { fOutputEvents->add<JPetEvent>(event); }
}

void PALSCalibrationTask::initialiseHistograms(){

  // General histograms
  getStatistics().createHistogramWithAxes(
    new TH2D("All_XYpos", "Hit position XY", 240, -60.25, 59.75, 240, -60.25, 59.75),
    "Hit X position [cm]", "Hit Y position [cm]"
  );
  
  std::map<int, JPetScin*> scinMap = getParamBank().getScintillators();
  auto minScinID = std::min_element(scinMap.begin(), scinMap.end(), [](const auto& l, const auto& r) { return l.first < r.first; });
  auto maxScinID = std::max_element(scinMap.begin(), scinMap.end(), [](const auto& l, const auto& r) { return l.first < r.first; });
  
  getStatistics().createHistogramWithAxes(
    new TH2D("TOT_vs_ID_matched", "TOT of the hit vs scintillator ID", 1200, -19500, 100500, maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5),
    "TOT of the hit [ps]", "Scintillator ID"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("Z_vs_ID", "Z position of the hit vs scintillator ID", 1000, -49.55, 50.45, maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5),
    "Z position [cm]", "Scintillator ID"
  );
  

  for (unsigned thrNum=1; thrNum<=fNmbOfThresholds; thrNum++) {
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("TDiff_AB_vs_ID_thr%d_calibrated", thrNum), Form("Time difference AB vs scintillator ID for threshold %d after calibration", thrNum), 
                                                500, -24750, 24250, maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5), 
                                                "Time difference AB [ps]", "ID of the scintillator"
    );
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("PALS_vs_AnnihilationID_thr%d", thrNum), Form("Positron Lifetime spectrum for a given Annihilation Hit ID for threshold %d", thrNum), 
                                                2*fEventTimeWindow/100, -fEventTimeWindow + 50, fEventTimeWindow + 50, //Bin size wil be always 0.1 ns
                                                maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5), 
                                                "Positron lifetime [ps]", "Annihilation Hit ID of the scintillator"
    );
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("PALS_vs_DeexcitationID_thr%d", thrNum), Form("Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold %d", thrNum), 
                                                2*fEventTimeWindow/100, -fEventTimeWindow + 50, fEventTimeWindow + 50, //Bin size wil be always 0.1 ns
                                                maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5), 
                                                "Positron lifetime [ps]", "Deexcitation Hit ID of the scintillator"
    );
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("Corrected_PALS_vs_AnnihilationID_thr%d", thrNum), 
                                                Form("Positron Lifetime spectrum for a given Annihilation Hit ID for threshold %d after correction", thrNum), 
                                                2*fEventTimeWindow/100, -fEventTimeWindow + 50, fEventTimeWindow + 50, //Bin size wil be always 0.1 ns
                                                maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5), 
                                                "Positron lifetime [ps]", "Annihilation Hit ID of the scintillator"
    );
    getStatistics().createHistogramWithAxes(
        new TH2D(Form("Corrected_PALS_vs_DeexcitationID_thr%d", thrNum), 
                                                Form("Positron Lifetime spectrum for a given Deexcitation Hit ID for threshold %d after correction", thrNum), 
                                                2*fEventTimeWindow/100, -fEventTimeWindow + 50, fEventTimeWindow + 50, //Bin size wil be always 0.1 ns
                                                maxScinID->first-minScinID->first+1, minScinID->first-0.5, maxScinID->first+0.5), 
                                                "Positron lifetime [ps]", "Deexcitation Hit ID of the scintillator"
    );
  }
  
  getStatistics().createHistogramWithAxes(
    new TH2D("XY_2Annihilation", "XY reconstructed position for 2 Annihilation Hits", 1000, -49.55, 50.45, 1000, -49.55, 50.45),
                                            "Hit horizontal position [cm] (Y - JPET)", "Hit vertical position [cm] (X -JPET)"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("Z_2Annihilation", "Z reconstructed position for 2 Annihilation Hits", 1000, -49.55, 50.45),
                                            "Hit longitudal position [cm] (Z - JPET)", "Counts"
  );
}

void PALSCalibrationTask::PlotTDiffAB_afterCalibration(JPetHit Hit)
{
  double TDiff_AB = 0.;
  std::map<int, double> sigALead = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int, double> sigBLead = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  unsigned ScintID = Hit.getScintillator().getID();
  double ABcorrection = 0.;
  
  std::vector<JPetSigCh> SigCh = Hit.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  if (sigALead.size()>0 && sigBLead.size()>0) {
    for (unsigned i=1; i<=sigALead.size() && i<=sigBLead.size(); i++) {
      if (sigBLead.find(i) != sigBLead.end() || sigALead.find(i) != sigALead.end())
        TDiff_AB = (sigBLead.find(i)->second - sigALead.find(i)->second);
      else
        TDiff_AB = 0;
      ABcorrection = UniversalFileLoader::getConfigurationParameter(fTimeCalibrationAB, SigCh.at(i-1).getDAQch());
      getStatistics().fillHistogram(Form("TDiff_AB_vs_ID_thr%d_calibrated", i), TDiff_AB - ABcorrection, ScintID);
    }
  }
}

void PALSCalibrationTask::PlotLifetimesForThresholds(JPetHit Hit1, JPetHit Hit2)
{
  double TDiff = 0.;
  std::map<int, double> sigALeadHit1 = Hit1.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int, double> sigBLeadHit1 = Hit1.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

  std::map<int, double> sigALeadHit2 = Hit2.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int, double> sigBLeadHit2 = Hit2.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);

  std::vector<JPetSigCh> SigCh1 = Hit1.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  std::vector<JPetSigCh> SigCh2 = Hit2.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  
  unsigned ScintID_1 = Hit1.getScintillator().getID();
  unsigned ScintID_2 = Hit2.getScintillator().getID();
  double POF1 = (Hit1.getPos() - fSourcePosition).Mag(); // Path of Fly
  double POF2 = (Hit2.getPos() - fSourcePosition).Mag();
  double TOFDiff = (POF1 - POF2)/kLightVelocity_cm_ps;
  double PALScorrection = 0., ABcorrection = 0.;

  if (sigALeadHit1.size() > 0 && sigBLeadHit1.size() > 0 && sigALeadHit2.size() > 0 && sigBLeadHit2.size() > 0) {
    for (unsigned i=1; i<=sigALeadHit1.size() && i<=sigBLeadHit1.size() && i<=sigALeadHit2.size() && i<=sigBLeadHit2.size(); i++) {
      TDiff = (sigBLeadHit1.find(i)->second + sigALeadHit1.find(i)->second)/2 - (sigBLeadHit2.find(i)->second + sigALeadHit2.find(i)->second)/2;  //TimeHit1 - TimeHit2
      ABcorrection = UniversalFileLoader::getConfigurationParameter(fTimeCalibrationAB, SigCh1.at(i-1).getDAQch())/2;
      ABcorrection -= UniversalFileLoader::getConfigurationParameter(fTimeCalibrationAB, SigCh2.at(i-1).getDAQch())/2;

      PALScorrection = UniversalFileLoader::getConfigurationParameter(fTimeCalibrationPALS, SigCh1.at(i-1).getDAQch())/2;
      PALScorrection -= UniversalFileLoader::getConfigurationParameter(fTimeCalibrationPALS, SigCh2.at(i-1).getDAQch())/2;

      getStatistics().fillHistogram(Form("PALS_vs_AnnihilationID_thr%d",i), TDiff - TOFDiff - ABcorrection, ScintID_1);
      getStatistics().fillHistogram(Form("PALS_vs_DeexcitationID_thr%d",i), TDiff - TOFDiff - ABcorrection, ScintID_2);
      getStatistics().fillHistogram(Form("Corrected_PALS_vs_AnnihilationID_thr%d",i), TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_1);
      getStatistics().fillHistogram(Form("Corrected_PALS_vs_DeexcitationID_thr%d",i), TDiff - TOFDiff - PALScorrection - ABcorrection, ScintID_2);
    }
  }
}

double PALSCalibrationTask::CorrectZPosition(JPetHit Hit)
{
  double TDiff = 0., ABcorrection = 0, velocity = 1;;
  std::map<int, double> sigALead = Hit.getSignalA().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  std::map<int, double> sigBLead = Hit.getSignalB().getRecoSignal().getRawSignal().getTimesVsThresholdNumber(JPetSigCh::Leading);
  
  std::vector<JPetSigCh> SigCh = Hit.getSignalB().getRecoSignal().getRawSignal().getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
  
  if (sigBLead.find(1) != sigBLead.end() || sigALead.find(1) != sigALead.end())
    TDiff = (sigBLead.find(1)->second - sigALead.find(1)->second);
  else
    TDiff = 0;
  ABcorrection = UniversalFileLoader::getConfigurationParameter(fTimeCalibrationAB, SigCh.at(0).getDAQch());
  velocity = UniversalFileLoader::getConfigurationParameter(fVelocityCalibration, SigCh.at(0).getDAQch());

  return (TDiff-ABcorrection)*velocity/2;
}
