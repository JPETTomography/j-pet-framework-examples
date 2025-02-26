/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
 *  @file Ntupler.cpp
 */

#include "NTupler.h"
#include "../ModularDetectorAnalysis/HitFinderTools.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

NTupler::NTupler(const char* name) : JPetUserTask(name) {}

bool NTupler::init()
{
  INFO("Started reduction of data to ntuples.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  if (isOptionSet(fParams.getOptions(), "inputFile_std::string"))
  {
    fOutFileName = getOptionAsString(fParams.getOptions(), "inputFile_std::string");
  }

  // initialize output file and tree
  if (fOutFileName.find("cat.evt.root") != std::string::npos)
  {
    fOutFileName.replace(fOutFileName.find("cat.evt.root"), std::string::npos, "ntu.root");
  }

  if (isOptionSet(fParams.getOptions(), "outputPath_std::string"))
  {
    fOutFilePath = getOptionAsString(fParams.getOptions(), "outputPath_std::string");
  }

  if (!fOutFilePath.empty())
  {
    size_t filename_pos = fOutFileName.find("dabc");
    fOutFileName.replace(0, filename_pos - 1, fOutFilePath);
  }

  fOutFile = new TFile(fOutFileName.c_str(), "RECREATE");
  fOutTree = new TTree("T", "JPET Events");

  fOutTree->SetBranchAddress("nhits", &fNumberOfHits);
  fOutTree->SetBranchAddress("times", &fHitTimes);
  fOutTree->SetBranchAddress("pos", &fHitPos);
  fOutTree->SetBranchAddress("tots", &fHitTOTs);
  fOutTree->SetBranchAddress("scins", &fHitScinIDs);

  return true;
}

bool NTupler::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {

    int n_events = timeWindow->getNumberOfEvents();

    for (int entry = 0; entry < n_events; ++entry)
    {
      const JPetEvent& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](entry));

      const auto& hits = event.getHits();
      fNumberOfHits = event.getHits().size();

      for (auto& hit : hits)
      {
        // Writing time in nanoseconds
        fHitTimes.push_back(hit->getTime() / 1000.);
        fHitPos.push_back(hit->getPos());
        fHitTOTs.push_back(hit->getEnergy());
        fHitScinIDs.push_back(hit->getScin().getID());
      }

      fOutTree->Fill();
      resetRow();
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool NTupler::terminate()
{
  fOutTree->Write();
  fOutFile->Close();

  INFO("Finished reduction of data to ntuples.");
  return true;
}

void NTupler::resetRow()
{
  fNumberOfHits = 0;
  fHitTimes.clear();
  fHitPos.clear();
  fHitTOTs.clear();
  fHitScinIDs.clear();
}
