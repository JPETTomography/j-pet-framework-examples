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
 *  @file Ntupler.h
 */

#ifndef NTUPLER_H 
#define NTUPLER_H 

#include <vector>
#include <map>
#include <string>
#include <JPetCommonTools/JPetCommonTools.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetEvent/JPetEvent.h>
//#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include <Hits/JPetBaseHit/JPetBaseHit.h>                                                                                                                                                   
#include <Hits/JPetMCRecoHit/JPetMCRecoHit.h> 

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

#ifdef __CINT__
#	define override
#endif

class NTupler : public JPetUserTask{

public:
  NTupler(const char * name);
  virtual ~NTupler(){}
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:  
  //double calculateTOTproportional(const JPetHit& hit) const;
  bool isThreeHitCluster(std::vector<const JPetBaseHit*> hits);
  void resetRow();
  
  bool fIsMC = false;  
  const double kSpeedOfLight = 29.9792458; // cm  / ns
  
  //std::string fThresholdValuesKey = "Ntupler_ThresholdValues_std::vector<int>";
  //std::vector<int> fThresholdValues;

  double kEventTimeWindow = 2500; //5000.0; //ps
  const std::string fEventTimeParamKey = "EventFinder_EventTime_float";
  //const std::string fEventTimeParamKey = "Ntupler_FineEventTime_float";
  
  TFile* fOutFile;
  TTree* fOutTree;
  std::string fOutFileName;
  std::string fOutFilePath;
  
  // ntuple components
  std::vector<double> fHitTimes;
  std::vector<TVector3> fHitPos;
  std::vector<UChar_t> fHitScinIDs;
  std::vector<double> fHitTOTsFlat;
  std::vector<double> fHitTOTs;

  UChar_t fNumberOfHits;
  
};
#endif /*  !NTUPLER_H */
