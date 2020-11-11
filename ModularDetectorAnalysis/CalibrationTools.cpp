/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file CalibrationTools.cpp
 */

using namespace std;

#include "CalibrationTools.h"
#include "TFile.h"

/**
 * Helper method
 */
void CalibrationTools::addHistograms(vector<TH1F*>& histograms, string fileName)
{
  // Get ROOT file with calbration histograms
  TFile* calibBankFile = new TFile(fileName.c_str(), "UPDATE");

  //Looping over histograms
  for(auto histo : histograms){
    // Check if already exists, if yes - add, no - write new
    if(calibBankFile->GetListOfKeys()->Contains(histo->GetName())){
      TH1F* old = dynamic_cast<TH1F*>(calibBankFile->Get(histo->GetName())->Clone());
      old->Add(histo);
      // overwriting existing histogram
      old->Write(0, 2);
    } else {
      histo->Write();
    }
  }
  calibBankFile->Close();
}
