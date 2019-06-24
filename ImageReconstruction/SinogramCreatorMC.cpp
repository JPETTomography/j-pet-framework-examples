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
 *  @file SinogramCreatorMC.cpp
 */

#include "SinogramCreatorMC.h"
#include "JPetSinogramType.h"
#include "JPetWriter/JPetWriter.h"
#include <TH1F.h>
#include <TH2I.h>

using namespace jpet_options_tools;

SinogramCreatorMC::SinogramCreatorMC(const char* name) : SinogramCreator(name) {}

SinogramCreatorMC::~SinogramCreatorMC() {}

bool SinogramCreatorMC::init()
{
  SinogramCreatorMC::setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  generateSinogram();
  return true;
}

void SinogramCreatorMC::generateSinogram()
{
  float firstX = 0.f;
  float firstY = 0.f;
  float secondX = 0.f;
  float secondY = 0.f;
  float firstZ = 0.f;
  float secondZ = 0.f;
  float firstT = 0.f;
  float secondT = 0.f;
  float skip = 0.f;
  int coincidence = 0;

  if (fSinogram == nullptr)
  {
    fSinogram = new JPetSinogramType::SparseMatrix*[fZSplitNumber];
    for (int i = 0; i < fZSplitNumber; i++)
    {
      fSinogram[i] = new JPetSinogramType::SparseMatrix(fMaxDistanceNumber, kReconstructionMaxAngle);
    }
  }

  for (const auto& inputPath : fInputData)
  {
    std::ifstream in(inputPath);
    while (in.peek() != EOF)
    {

      in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT >> skip >> skip >> skip >> skip >> coincidence >> skip >>
          skip >> skip; // read goja data type: https://github.com/JPETTomography/j-pet-gate-tools/tree/master/goja
      // in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT;

      if (coincidence != 1) // 1 == true event
        continue;

      if (analyzeHits(TVector3(firstX, firstY, firstZ), firstT, TVector3(secondX, secondY, secondZ), secondT))
      {
        fNumberOfCorrectHits++;
        // if(numberOfCorrectHits % 10000 == 0)
        //  break;
      }
      fTotalHits++;
    }
  }
  if (fTotalHits == 0)
    fTotalHits = 1; // to do not divide by 0
  std::cout << "Correct hits: " << fNumberOfCorrectHits << " total hits: " << fTotalHits
            << " (correct percentage: " << (((float)fNumberOfCorrectHits * 100.f) / (float)fTotalHits) << "%)" << std::endl;
}

bool SinogramCreatorMC::exec() { return true; }

bool SinogramCreatorMC::terminate()
{
  // Save sinogram to root file.
  JPetSinogramType map("Sinogram", fZSplitNumber, fMaxDistanceNumber, fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy,
                       fScintillatorLenght, 10000, 10000, fZSplitRange);
  for (int k = 0; k < fZSplitNumber; k++)
  {
    map.addSlice((*fSinogram[k]), k);
  }
  map.setNumberOfAllEvents(fTotalHits);
  map.setNumberOfEventsUsedToCreateSinogram(fNumberOfCorrectHits);
  JPetWriter* writer = new JPetWriter(fOutFileName.c_str());
  map.saveSinogramToFile(writer);
  writer->closeFile();

  delete[] fSinogram;
  return true;
}

void SinogramCreatorMC::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) { fOutFileName = getOptionAsString(opts, kOutFileNameKey); }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy))
  { fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy); }
  if (isOptionSet(opts, kZSplitNumber)) { fZSplitNumber = getOptionAsInt(opts, kZSplitNumber); }
  if (isOptionSet(opts, kScintillatorLenght)) { fScintillatorLenght = getOptionAsFloat(opts, kScintillatorLenght); }
  if (isOptionSet(opts, kMaxReconstructionRadius)) { fMaxReconstructionLayerRadius = getOptionAsFloat(opts, kMaxReconstructionRadius); }
  if (isOptionSet(opts, kInputDataKey)) { fInputData = getOptionAsVectorOfStrings(opts, kInputDataKey); }
  if (isOptionSet(opts, kEnableObliqueLORRemapping)) { fEnableObliqueLORRemapping = getOptionAsBool(opts, kEnableObliqueLORRemapping); }
  if (isOptionSet(opts, kEnableTOFReconstruction)) { fEnableKDEReconstruction = getOptionAsBool(opts, kEnableTOFReconstruction); }

  fTOFInformation = new JPetRecoImageTools::Matrix2DTOF[fZSplitNumber];
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
