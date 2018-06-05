/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file SinogramCreator.cpp
 */

#include "SinogramCreator.h"
#include <TH2I.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TH1I.h>
#include <RVersion.h>
using namespace jpet_options_tools;

SinogramCreator::SinogramCreator(const char *name) : JPetUserTask(name) {}

SinogramCreator::~SinogramCreator() {}

bool SinogramCreator::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH2I("reconstuction_histogram",
                                           "reconstuction histogram",
                                           std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, 0.f, fMaxReconstructionLayerRadius,
                                           kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(new TH1F("pos_dis", "Position distance real data", (fMaxReconstructionLayerRadius)*10 * 5, 0.f, fMaxReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("angle", "Position angle real data", kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
  getStatistics().getObject<TH2I>("reconstuction_histogram")->SetBit(TH2::kCanRebin);
  getStatistics().getObject<TH1F>("angle")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_dis")->SetBit(TH1::kCanRebin);
#else
  getStatistics().getObject<TH2I>("reconstuction_histogram")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("angle")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("pos_dis")->SetCanExtend(TH1::kAllAxes);
#endif

  return true;
}

bool SinogramCreator::exec()
{
  const int maxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr)
  {
    fSinogram = new SinogramResultType *[fZSplitNumber];
    for (int i = 0; i < fZSplitNumber; i++)
    {
      fSinogram[i] = new SinogramResultType(maxDistanceNumber, (std::vector<unsigned int>(kReconstructionMaxAngle, 0)));
    }
  }
  if (const auto &timeWindow = dynamic_cast<const JPetTimeWindow *const>(fEvent))
  {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      const auto event = dynamic_cast<const JPetEvent &>(timeWindow->operator[](static_cast<int>(i)));
      const auto hits = event.getHits();
      if (hits.size() != 2)
      {
        continue;
      }
      const auto &firstHit = hits[0];
      const auto &secondHit = hits[1];
      const float firstX = firstHit.getPosX();
      const float firstY = firstHit.getPosY();
      const float secondX = secondHit.getPosX();
      const float secondY = secondHit.getPosY(); // copy positions

      const float firstZ = firstHit.getPosZ();
      const float secondZ = secondHit.getPosZ();

      for (int i = 0; i < fZSplitNumber; i++)
      {
        if (!checkSplitRange(firstZ, secondZ, i))
        {
          continue;
        }

        const float distance = SinogramCreatorTools::calculateDistance(firstX, firstY, secondX, secondY);
        getStatistics().getObject<TH1F>("pos_dis")->Fill(distance);
        const int angle = SinogramCreatorTools::calculateAngle(firstX, firstY, secondX, secondY);

        getStatistics().getObject<TH1F>("angle")->Fill(angle);
        const int distanceRound = SinogramCreatorTools::roundToNearesMultiplicity(distance + fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy);
        if (distanceRound >= maxDistanceNumber || angle >= kReconstructionMaxAngle)
        {
          std::cout << "Distance round: " << distanceRound << " angle: " << angle << std::endl;
          continue;
        }
        fCurrentValueInSinogram[i] = ++fSinogram[i]->at(distanceRound).at(angle);
        if (fCurrentValueInSinogram[i] > fMaxValueInSinogram[i])
        {
          fMaxValueInSinogram[i] = fCurrentValueInSinogram[i]; // save max value of sinogram
        }
        getStatistics().getObject<TH2I>("reconstuction_histogram")->Fill(distance + fMaxReconstructionLayerRadius, angle); //add to histogram
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

bool SinogramCreator::checkSplitRange(float firstZ, float secondZ, int i)
{
  return firstZ >= fZSplitRange[i].first && firstZ <= fZSplitRange[i].second && secondZ >= fZSplitRange[i].first && secondZ <= fZSplitRange[i].second;
}

bool SinogramCreator::terminate()
{
  for (int i = 0; i < fZSplitNumber; i++)
  {
    std::ofstream res(fOutFileName + std::to_string(i) + ".ppm");
    res << "P2" << std::endl;
    res << (*fSinogram[i])[0].size() << " " << fSinogram[i]->size() << std::endl;
    res << fMaxValueInSinogram[i] << std::endl;
    for (unsigned int k = 0; k < fSinogram[i]->size(); k++)
    {
      for (unsigned int j = 0; j < (*fSinogram[i])[0].size(); j++)
      {
        res << (*fSinogram[i])[k][j] << " ";
      }
      res << std::endl;
    }
    res.close();
  }
  delete[] fSinogram;
  delete[] fMaxValueInSinogram;

  return true;
}

void SinogramCreator::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey))
  {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy))
  {
    fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy);
  }

  if (isOptionSet(opts, kZSplitNumber))
  {
    fZSplitNumber = getOptionAsInt(opts, kZSplitNumber);
  }

  if (isOptionSet(opts, kScintillatorLenght))
  {
    fScintillatorLenght = getOptionAsFloat(opts, kScintillatorLenght);
  }

  const JPetParamBank bank = getParamBank();
  const JPetGeomMapping mapping(bank);
  fMaxReconstructionLayerRadius = mapping.getRadiusOfLayer(mapping.getLayersCount());

  fMaxValueInSinogram = new int[fZSplitNumber];
  fCurrentValueInSinogram = new int[fZSplitNumber];
  const float maxZRange = fScintillatorLenght / 2.f;
  float range = (2.f * maxZRange) / fZSplitNumber;
  for (int i = 0; i < fZSplitNumber; i++)
  {
    float rangeStart = (i * range) - maxZRange;
    float rangeEnd = ((i + 1) * range) - maxZRange;
    fZSplitRange.push_back(std::make_pair(rangeStart, rangeEnd));
    fCurrentValueInSinogram[i] = 0;
  }
}
