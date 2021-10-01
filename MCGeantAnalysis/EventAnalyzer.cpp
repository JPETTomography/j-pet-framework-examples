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
 *  @file EventAnalyzer.cpp
 */

#include "EventAnalyzer.h"
#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include <Hits/JPetMCRecoHit/JPetMCRecoHit.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetRawMCHit/JPetRawMCHit.h>

using namespace jpet_options_tools;
using namespace std;

EventAnalyzer::EventAnalyzer(const char* name) : JPetUserTask(name) {}

EventAnalyzer::~EventAnalyzer() {}

bool EventAnalyzer::init()
{
  INFO("Event analysis started.");

  getStatistics().createHistogramWithAxes(new TH1D("z_res", "Resolution along Z", 301, -15.05, 15.05), "Z_{REC}-Z_{MC} [cm]");

  getStatistics().createHistogramWithAxes(new TH1D("Edep_res", "Resolution of deposited energy", 201, -201., 201.), "E_{REC}-E_{MC} [keV]");

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  return true;
}

bool EventAnalyzer::exec()
{

  // Identify whether the input events are MC or DATA.
  // In case of MC, store the pointer to the TimeWindowMC object
  // which contains "true MC" information about the generated events.
  JPetTimeWindowMC* time_window_mc = nullptr;
  if (time_window_mc = dynamic_cast<JPetTimeWindowMC* const>(fEvent))
  {
    fIsMC = true;
    INFO("The input file is MC.");
  }
  else
  {
    INFO("The input file is DATA.");
  }

  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {

    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      // if the input is MC, we fill resolution histograms
      // to check if MC smearing works fine
      if (fIsMC)
      {
        fillResolutionHistograms(event, time_window_mc);
      }
    }
  }
  else
  {
    return false;
  }

  return true;
}

bool EventAnalyzer::terminate()
{
  INFO("Event analysis completed.");
  return true;
}

void EventAnalyzer::fillResolutionHistograms(const JPetEvent& event, const JPetTimeWindowMC* tw)
{

  int hits_number = event.getHits().size();
  for (int k = 0; k < hits_number; ++k)
  {
    auto reconstructed_hit = dynamic_cast<const JPetMCRecoHit*>(event.getHits().at(k));
    if (!reconstructed_hit)
    {
      continue;
    }
    // for each reconstructed hit, we access the corresponding "true MC" hit
    const JPetRawMCHit& mc_hit = tw->getMCHit<JPetRawMCHit>(reconstructed_hit->getMCindex());

    getStatistics().fillHistogram("z_res", reconstructed_hit->getPos().Z() - mc_hit.getPos().Z());
    getStatistics().fillHistogram("Edep_res", reconstructed_hit->getEnergy() - mc_hit.getEnergy());
  }
}
