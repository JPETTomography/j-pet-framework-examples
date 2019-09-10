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
#include <TH1I.h>
#include <TH2F.h>
#include <TH2I.h>
#include <TH3F.h>
using namespace jpet_options_tools;

SinogramCreator::SinogramCreator(const char* name) : JPetUserTask(name) {}

SinogramCreator::~SinogramCreator() {}

bool SinogramCreator::init()
{
  setUpOptions();

  fOutputEvents = new JPetTimeWindow("JPetEvent");
  fSinogramData = JPetSinogramType::WholeSinogram(fZSplitNumber, JPetSinogramType::Matrix3D());

  if (!fGojaInputFilePath.empty())
  {
    readAndAnalyzeGojaFile();
  }

  return true;
}

void SinogramCreator::readAndAnalyzeGojaFile()
{
  float firstX = 0.f;
  float firstY = 0.f;
  float secondX = 0.f;
  float secondY = 0.f;
  float firstZ = 0.f;
  float secondZ = 0.f;
  double firstT = 0.f;
  double secondT = 0.f;

  float sourceX = 0.f;
  float sourceY = 0.f;
  float sourceZ = 0.f;

  float skip = 0.f;
  float skipInt = 0.f;
  int coincidence = 0;

  for (const auto& inputPath : fGojaInputFilePath)
  {
    std::ifstream in(inputPath);
    while (in.peek() != EOF)
    {
      in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT >> skipInt >> skipInt >> skip >> skip >> coincidence >> skip >>
          skip >> skip;
       in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT;
      //if (coincidence != 1) // 1 == true event
      //  continue;

      fTotalAnalyzedHits++;

      if (fEnableNEMAAttenuation)
      {
        if (firstZ - secondZ < 30. &&
            atenuation(SinogramCreatorTools::getPolyFit(std::vector<double>{(std::sqrt(sourceX * sourceX + sourceY * sourceY), -std::abs(sourceZ))})))
        {
          fTotalAttenuatedHits++;
          continue;
        }
      }

      if (analyzeHits(firstX, firstY, firstZ, firstT, secondX, secondY, secondZ, secondT))
      {
        fNumberOfCorrectHits++;
      }
    }
  }
}

bool SinogramCreator::exec()
{
  if (!fGojaInputFilePath.empty())
  {
    return true;
  }

  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      const auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      const auto hits = event.getHits();
      if (hits.size() != 2)
      {
        continue;
      }
      const auto& firstHit = hits[0];
      const auto& secondHit = hits[1];
      if (analyzeHits(firstHit, secondHit))
        fNumberOfCorrectHits++;
      fTotalAnalyzedHits++;
    }
  }
  else
  {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool SinogramCreator::analyzeHits(const JPetHit& firstHit, const JPetHit& secondHit)
{
  return analyzeHits(firstHit.getPos(), firstHit.getTime(), secondHit.getPos(), secondHit.getTime());
}

bool SinogramCreator::analyzeHits(const float firstX, const float firstY, const float firstZ, const double firstTOF, const float secondX,
                                  const float secondY, const float secondZ, const double secondTOF)
{
  int i = -1;
  if (!fEnableObliqueLORRemapping)
  {
    i = SinogramCreatorTools::getSplitRangeNumber(firstZ, secondZ, fZSplitRange);
  }
  else
  {
    i = SinogramCreatorTools::getSinogramSlice(firstX, firstY, firstZ, firstTOF, secondX, secondY, secondZ, secondTOF, fZSplitRange);
  }
  if (i < 0 || i >= fZSplitNumber)
  {
    WARNING("WARNING, reconstructed sinogram slice on 'z' is out of range: " + std::to_string(i) +
            " max slice number: " + std::to_string(fZSplitNumber));
    return false;
  }
  const auto sinogramResult = SinogramCreatorTools::getSinogramRepresentation(
      firstX, firstY, secondX, secondY, fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy, fMaxDistanceNumber, kReconstructionMaxAngle);
  const auto TOFSlice = SinogramCreatorTools::getTOFSlice(firstTOF, secondTOF, fTOFBinSliceSize);
  const auto data = fSinogramData[i].find(TOFSlice);
  if (sinogramResult.first >= fMaxDistanceNumber || sinogramResult.second >= kReconstructionMaxAngle)
    return false;
  if (data != fSinogramData[i].end())
  {
    data->second(sinogramResult.first, sinogramResult.second) += 1.;
  }
  else
  {
    fSinogramData[i].insert(std::make_pair(
        TOFSlice, JPetSinogramType::SparseMatrix(fMaxDistanceNumber, kReconstructionMaxAngle, fMaxDistanceNumber * kReconstructionMaxAngle)));
    fSinogramData[i][TOFSlice](sinogramResult.first, sinogramResult.second) += 1.;
  }

  return true;
}

bool SinogramCreator::analyzeHits(const TVector3& firstHit, const float firstTOF, const TVector3& secondHit, const float secondTOF)
{
  return analyzeHits(firstHit.X(), firstHit.Y(), firstHit.Z(), firstTOF, secondHit.X(), secondHit.Y(), secondHit.Z(), secondTOF);
}

float SinogramCreator::getTOFRescaleFactor(const TVector3& posDiff) const
{
  float distance_3d = std::sqrt(posDiff.X() * posDiff.X() + posDiff.Y() * posDiff.Y() + posDiff.Z() * posDiff.Z());
  float distance_2d = std::sqrt(posDiff.X() * posDiff.X() + posDiff.Y() * posDiff.Y());
  if (distance_3d < kEPSILON)
    return 0.f;
  return distance_2d / distance_3d;
}

bool SinogramCreator::terminate()
{
  // Save sinogram to root file.
  JPetSinogramType map("Sinogram", fZSplitNumber, fMaxDistanceNumber, fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy,
                       fScintillatorLenght, fTOFBinSliceSize, fZSplitRange);
  map.addSinogram(fSinogramData);
  map.setNumberOfAllEvents(fTotalAnalyzedHits);
  map.setNumberOfEventsUsedToCreateSinogram(fNumberOfCorrectHits);
  JPetWriter* writer = new JPetWriter(fOutFileName.c_str());
  map.saveSinogramToFile(writer);
  writer->closeFile();

  float totalCorrectProcentage = 0.f;
  if (fTotalAnalyzedHits != 0)
    totalCorrectProcentage = (((float)fNumberOfCorrectHits * 100.f) / (float)fTotalAnalyzedHits);
  std::cout << "Correct hits: " << fNumberOfCorrectHits << " total hits: " << fTotalAnalyzedHits << " (correct percentage: " << totalCorrectProcentage
            << "%)"
            << " attenuated: " << fTotalAttenuatedHits << std::endl
            << std::endl;

  return true;
}

bool SinogramCreator::atenuation(const float value) { return distribution(generator) <= value; }

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
  if (isOptionSet(opts, kEnableObliqueLORRemapping))
  {
    fEnableObliqueLORRemapping = getOptionAsBool(opts, kEnableObliqueLORRemapping);
  }
  if (isOptionSet(opts, kEnableTOFReconstrution))
  {
    fEnableKDEReconstruction = getOptionAsBool(opts, kEnableTOFReconstrution);
  }
  if (isOptionSet(opts, kGojaInputFilePath))
  {
    fGojaInputFilePath = getOptionAsVectorOfStrings(opts, kGojaInputFilePath);
  }
  if (isOptionSet(opts, kEnableNEMAAttenuation))
  {
    fEnableNEMAAttenuation = getOptionAsBool(opts, kEnableNEMAAttenuation);
  }
  if (isOptionSet(opts, kTOFBinSliceSize))
  {
    fTOFBinSliceSize = getOptionAsFloat(opts, kTOFBinSliceSize);
  }

  if (isOptionSet(opts, kMaxReconstructionLayerRadius))
  {
    fMaxReconstructionLayerRadius = getOptionAsFloat(opts, kMaxReconstructionLayerRadius);
  }
  else
  {
    const JPetParamBank& bank = getParamBank();
    const JPetGeomMapping mapping(bank);
    fMaxReconstructionLayerRadius = mapping.getRadiusOfLayer(mapping.getLayersCount() - 1);
  }

  const float maxZRange = fScintillatorLenght / 2.f;
  float range = (2.f * maxZRange) / fZSplitNumber;
  for (int i = 0; i < fZSplitNumber; i++)
  {
    float rangeStart = (i * range) - maxZRange;
    float rangeEnd = ((i + 1) * range) - maxZRange;
    fZSplitRange.push_back(std::make_pair(rangeStart, rangeEnd));
  }

  fMaxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
}
