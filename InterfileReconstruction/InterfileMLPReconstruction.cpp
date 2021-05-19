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
 */
#include "InterfileMLPReconstruction.h"
#include "InterfileReconstructionTools.h"
#include "EventCategorizerTools.h"
#include <TH3D.h>
#include <TVector3.h>
#include "JPetOptionsTools/JPetOptionsTools.h"
#include "JPetEvent/JPetEvent.h"
using namespace jpet_options_tools;

InterfileMLPReconstruction::InterfileMLPReconstruction(const char *name) : JPetUserTask(name) {}

InterfileMLPReconstruction::~InterfileMLPReconstruction() {}

bool InterfileMLPReconstruction::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH3D("hits_pos",
                                           "Reconstructed hit pos",
                                           fNumberOfBinsX, -fXRange, fXRange,
                                           fNumberOfBinsY, -fYRange, fYRange,
                                           fNnumberOfBinsZ, -fZRange, fZRange));
  return true;
}

bool InterfileMLPReconstruction::exec()
{
  if (const auto &timeWindow = dynamic_cast<const JPetTimeWindow *const>(fEvent))
  {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      auto event = dynamic_cast<const JPetEvent &>(timeWindow->operator[](static_cast<int>(i)));
      auto numberOfHits = event.getHits().size();
      if (numberOfHits <= 1)
        continue;
      else
      {
        auto hits = event.getHits();
        for (unsigned int i = 0; i < hits.size() - 1; i++)
        {
          for (unsigned int j = i + 1; j < hits.size(); j++) {
            TVector3 annihilationPoint = EventCategorizerTools::calculateAnnihilationPoint(hits[i], hits[j]);
            getStatistics().getObject<TH3D>("hits_pos")->Fill(annihilationPoint.X(), annihilationPoint.Y(), annihilationPoint.Z());
          }
        }
      }
    }
  }
  else
  {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool InterfileMLPReconstruction::terminate()
{
  InterfileReconstructionTools::saveToInterfile(getStatistics().getObject<TH3D>("hits_pos"), fOutputPath);
  return true;
}

void InterfileMLPReconstruction::setUpOptions()
{
  auto opts = getOptions();

  if (isOptionSet(opts, kOutputPath))
  {
    fOutputPath = getOptionAsString(opts, kOutputPath);
  }

  if (isOptionSet(opts, kXRangeOn3DHistogramKey))
  {
    fXRange = getOptionAsInt(opts, kXRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kYRangeOn3DHistogramKey))
  {
    fYRange = getOptionAsInt(opts, kYRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kZRangeOn3DHistogramKey))
  {
    fZRange = getOptionAsInt(opts, kZRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kBinMultiplierKey))
  { //sets-up bin size in root 3d-histogram, root cannot write more then 1073741822 bytes to 1 histogram
    const int kMaxRootFileSize = 1073741822;
    fBinMultiplier = getOptionAsDouble(opts, kBinMultiplierKey);
    if ((std::floor(fBinMultiplier * fXRange) *
         std::floor(fBinMultiplier * fYRange) *
         std::floor(fBinMultiplier * fZRange)) > kMaxRootFileSize)
    {
      fBinMultiplier = 6;
      WARNING("TBufferFile can only write up to 1073741822 bytes, bin multiplier is too big, reseted to 6");
    }
    fNumberOfBinsX = fXRange * fBinMultiplier;
    fNumberOfBinsY = fYRange * fBinMultiplier;
    fNnumberOfBinsZ = fZRange * fBinMultiplier;
  }
}
