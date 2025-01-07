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
 *  @file Downscaler.cpp
 */

#include "Downscaler.h"
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <algorithm>
#include <iostream>

using namespace jpet_options_tools;
using namespace std;

Downscaler::Downscaler(const char* name) : JPetUserTask(name)
{
  std::random_device rd;
  fRandomGenerator = std::mt19937(rd());
}

Downscaler::~Downscaler() {}

bool Downscaler::init()
{
  INFO("Event downscaling started.");

  // Parameter for back to back categorization
  if (isOptionSet(fParams.getOptions(), fDownscalingRatesKey))
  {
    fDownscalingRates = getOptionAsVectorOfDoubles(fParams.getOptions(), fDownscalingRatesKey);
    std::transform(fDownscalingRates.begin(), fDownscalingRates.end(), fDownscalingRates.begin(), [](double el) -> double { return el / 100.; });
  }
  else
  {
    WARNING("No downscaling rates provided by the user. All events will be "
            "passed on at a 100% rate.");
  }

  getStatistics().createHistogramWithAxes(new TH1D("filtered_event_multiplicity", "Number of hits in filtered events", 20, 0.5, 20.5),
                                          "Hits in Event", "Number of Hits");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  return true;
}

bool Downscaler::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));

      int multiplicity = event.getHits().size();
      double rate = 1.0;
      if (multiplicity <= fDownscalingRates.size())
      {
        rate = fDownscalingRates[multiplicity - 1];
        if (getNormalizedRandom() < rate)
        {
          saveEvent(event);
        }
      }
      else
      { // rate not specified for this multiplicity
        saveEvent(event);
      }
    }
  }
  else
  {
    return false;
  }
  return true;
}

bool Downscaler::terminate()
{
  INFO("Event downscaling completed.");
  return true;
}

double Downscaler::getNormalizedRandom() { return fRandomDistribution(fRandomGenerator); }

void Downscaler::saveEvent(const JPetEvent& event)
{
  getStatistics().fillHistogram("filtered_event_multiplicity", event.getHits().size());
  fOutputEvents->add<JPetEvent>(event);
}
