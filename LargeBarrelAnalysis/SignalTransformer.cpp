/**
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
 *  @file SignalTransformer.cpp
 */

#include "JPetWriter/JPetWriter.h"
#include "SignalTransformer.h"

using namespace jpet_options_tools;

SignalTransformer::SignalTransformer(const char* name): JPetUserTask(name) {}

SignalTransformer::~SignalTransformer() {}

bool SignalTransformer::init()
{
  INFO("Signal transforming started: Raw to Reco and Phys");
  fOutputEvents = new JPetTimeWindow("JPetPhysSignal");
  // Getting bool for using bad signals
  if (isOptionSet(fParams.getOptions(), kUseCorruptedSignalsParamKey)) {
    fUseCorruptedSignals = getOptionAsBool(fParams.getOptions(), kUseCorruptedSignalsParamKey);
    if(fUseCorruptedSignals){
      WARNING("Signal Transformer is using Corrupted Signals, as set by the user");
    } else{
      WARNING("Signal Transformer is NOT using Corrupted Signals, as set by the user");
    }
  } else {
    WARNING("Signal Transformer is not using Corrupted Signals (default option)");
  }
  //Walk correction constants (for each threshold separately)
  if (isOptionSet(fParams.getOptions(), kWalkCorrConst1ParamKey)) {
    fWalkCorrConst[0] = getOptionAsFloat(fParams.getOptions(), kWalkCorrConst1ParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kWalkCorrConst2ParamKey)) {
    fWalkCorrConst[1] = getOptionAsFloat(fParams.getOptions(), kWalkCorrConst2ParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kWalkCorrConst3ParamKey)) {
    fWalkCorrConst[2] = getOptionAsFloat(fParams.getOptions(), kWalkCorrConst3ParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kWalkCorrConst4ParamKey)) {
    fWalkCorrConst[3] = getOptionAsFloat(fParams.getOptions(), kWalkCorrConst4ParamKey);
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool SignalTransformer::exec()
{
  if(auto & timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();
    for(uint i=0;i<n;++i){
      auto& rawSignal = dynamic_cast<const JPetRawSignal&>(timeWindow->operator[](i));
      if(!fUseCorruptedSignals && rawSignal.getRecoFlag()==JPetBaseSignal::Corrupted) {
        continue;
      }
      if(fSaveControlHistos) {
        auto leads = rawSignal.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrNum);
        auto trails = rawSignal.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrNum);
        for(unsigned int i=0;i<leads.size();i++){
          getStatistics().getHisto1D("raw_sigs_multi")->Fill(2*i+1);
        }
        for(unsigned int i=0;i<trails.size();i++){
          getStatistics().getHisto1D("raw_sigs_multi")->Fill(2*(i+1));
        }
        if(rawSignal.getRecoFlag()==JPetBaseSignal::Good){
          getStatistics().getHisto1D("good_vs_bad_signals")->Fill(1);
          for(unsigned int i=0;i<leads.size();i++){
            getStatistics().getHisto1D("raw_sigs_multi_good")->Fill(2*i+1);
          }
          for(unsigned int i=0;i<trails.size();i++){
            getStatistics().getHisto1D("raw_sigs_multi_good")->Fill(2*(i+1));
          }
        } else if(rawSignal.getRecoFlag()==JPetBaseSignal::Corrupted){
	  //
	  int PMid = leads.at(0).getPM().getID();
	  
	  getStatistics().getHisto1D("PmIdCorrupted")->Fill(PMid);
          getStatistics().getHisto1D("good_vs_bad_signals")->Fill(2);
          for(unsigned int i=0;i<leads.size();i++){
            getStatistics().getHisto1D("raw_sigs_multi_corr")->Fill(2*i+1);
            if(leads.at(i).getRecoFlag()==JPetSigCh::Good){
              getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->Fill(2*i+1);
            } else if(leads.at(i).getRecoFlag()==JPetSigCh::Corrupted){
              getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->Fill(2*i+1);
            }
          }
          for(unsigned int i=0;i<trails.size();i++){
            getStatistics().getHisto1D("raw_sigs_multi_corr")->Fill(2*(i+1));
            if(trails.at(i).getRecoFlag()==JPetSigCh::Good){
              getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->Fill(2*(i+1));
            } else if(trails.at(i).getRecoFlag()==JPetSigCh::Corrupted){
              getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->Fill(2*(i+1));
            }
          }
        } else if(rawSignal.getRecoFlag()==JPetBaseSignal::Unknown){
          getStatistics().getHisto1D("good_vs_bad_signals")->Fill(3);
        }
      }
      // Make Reco Signal from Raw Signal
      auto recoSignal = createRecoSignal(rawSignal);
      // Make Phys Signal from Reco Signal and save
      auto physSignal = createPhysSignal(recoSignal);
      fOutputEvents->add<JPetPhysSignal>(physSignal);
    }
  } else {
    return false;
  }
  return true;
}

bool SignalTransformer::terminate()
{
  INFO("Signal transforming finished");
  return true;
}

/**
 * Method rewrites Raw Signal to Reco Signal. All fields set to -1.
 */
JPetRecoSignal SignalTransformer::createRecoSignal(const JPetRawSignal& rawSignal)
{
  JPetRecoSignal recoSignal;  
  recoSignal.setRawSignal(rawSignal);
  recoSignal.setAmplitude(-1.0);
  recoSignal.setOffset(-1.0);
  recoSignal.setCharge(-1.0);
  recoSignal.setDelay(-1.0);
  recoSignal.setRecoFlag(rawSignal.getRecoFlag());
  return recoSignal;
}

/**
 * Method rewrites Reco Signal to Phys Signal.
 * Time of Signal set to time of the Leading Signal Channel at the lowest threshold.
 * Other fields are set to -1, quality fields set to 0.
 */
JPetPhysSignal SignalTransformer::createPhysSignal(const JPetRecoSignal& recoSignal)
{
  JPetPhysSignal physSignal;
  correctForWalk(recoSignal);
  std::vector<JPetSigCh> leadingSigChVec = recoSignal.getRawSignal().getPoints(
       JPetSigCh::Leading, JPetRawSignal::ByThrValue
									       );
  physSignal.setRecoSignal(recoSignal);
  physSignal.setPhe(-1.0);
  physSignal.setQualityOfPhe(0.0);
  physSignal.setQualityOfTime(0.0);
  physSignal.setRecoFlag(recoSignal.getRecoFlag());
  physSignal.setTime(leadingSigChVec.at(0).getValue());
  return physSignal;
}
//Walk correction applyed to the SigCh times on both edges
//
void SignalTransformer::correctForWalk(const JPetRecoSignal& recoSignal)
{
  std::vector<JPetSigCh> leadingSigChVec = recoSignal.getRawSignal().getPoints(
       JPetSigCh::Leading, JPetRawSignal::ByThrValue);
  std::vector<JPetSigCh> trailingSigChVec = recoSignal.getRawSignal().getPoints(
       JPetSigCh::Trailing, JPetRawSignal::ByThrValue);
  double TOT = 0.;
  for (unsigned i = 0; i < leadingSigChVec.size() && i < trailingSigChVec.size(); i++) {
    TOT += trailingSigChVec.at(i).getValue() - leadingSigChVec.at(i).getValue();
  }
   for (unsigned i = 0; i < leadingSigChVec.size();i++){
     if(TOT>0. && fWalkCorrConst[i] >0.){
       double WalkCorr = fWalkCorrConst[i]/sqrt(TOT);
       leadingSigChVec.at(i).setValue(leadingSigChVec.at(i).getValue() - WalkCorr);
       getStatistics().getHisto1D("WalkCorrLead")->Fill(WalkCorr);
     }
   for (unsigned i = 0; i < trailingSigChVec.size();i++){
     if(TOT>0. && fWalkCorrConst[i] >0.){
       double WalkCorr = fWalkCorrConst[i]/sqrt(TOT);
       trailingSigChVec.at(i).setValue(trailingSigChVec.at(i).getValue() - WalkCorr);
       getStatistics().getHisto1D("WalkCorrTrail")->Fill(WalkCorr);
     }
   }
  }
}
void SignalTransformer::initialiseHistograms(){
  getStatistics().createHistogram(
    new TH1F("good_vs_bad_signals", "Number of good and corrupted signals created", 3, 0.5, 3.5)
  );
  getStatistics().getHisto1D("good_vs_bad_signals")->GetXaxis()->SetBinLabel(1,"GOOD");
  getStatistics().getHisto1D("good_vs_bad_signals")->GetXaxis()->SetBinLabel(2,"CORRUPTED");
  getStatistics().getHisto1D("good_vs_bad_signals")->GetXaxis()->SetBinLabel(3,"UNKNOWN");
  getStatistics().getHisto1D("good_vs_bad_signals")->GetYaxis()->SetTitle("Number of Signals");

  getStatistics().createHistogram(new TH1F(
    "raw_sigs_multi", "Multiplicity of created Raw Signals",
    8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("raw_sigs_multi")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("raw_sigs_multi")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "raw_sigs_multi_good", "Multiplicity of created Raw Signals with GOOD flag",
    8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_good")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "raw_sigs_multi_corr", "Multiplicity of created Raw Signals with CORRUPTED flag",
    8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "raw_sigs_multi_corr_sigch_good", "Multiplicity of created Raw Signals with CORRUPTED flag - GOOD SigCh olny",
    8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_good")->GetYaxis()->SetTitle("Number of GOOD SigChs");

  getStatistics().createHistogram(new TH1F(
    "raw_sigs_multi_corr_sigch_corr", "Multiplicity of created Raw Signals with CORRUPTED flag - CORRUPTED SigCh olny",
    8, 0.5, 8.5
  ));
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(5,"THR 3 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(6,"THR 3 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(7,"THR 4 Lead");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetXaxis()->SetBinLabel(8,"THR 4 Trail");
  getStatistics().getHisto1D("raw_sigs_multi_corr_sigch_corr")->GetYaxis()->SetTitle("Number of CORRUPTED SigChs");

  getStatistics().createHistogram(new TH1F(
					   "WalkCorrLead", "Walk Correction applied on the leading edge",
					   1000, 0.,50.
  ));
  getStatistics().getHisto1D("WalkCorrLead")->GetXaxis()->SetTitle("Walk Correction [ps]");
  getStatistics().getHisto1D("WalkCorrLead")->GetYaxis()->SetTitle("Counts");
  getStatistics().getHisto1D("WalkCorrLead")->GetYaxis()->SetTitle("Walk Correction applied on the leading edge");

  getStatistics().createHistogram(new TH1F(
                                           "WalkCorrTrail", "Walk Correction applied on the trailing edge",
                                           1000, 0.,50.
					   ));
  getStatistics().getHisto1D("WalkCorrTrail")->GetXaxis()->SetTitle("Walk Correction [ps]");
  getStatistics().getHisto1D("WalkCorrTrail")->GetYaxis()->SetTitle("Counts");
  getStatistics().getHisto1D("WalkCorrTrail")->GetYaxis()->SetTitle("Walk Correction applied on the trailing edge");
}
