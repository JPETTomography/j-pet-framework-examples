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
#include <TH1F.h>
#include <TH2I.h>
using namespace jpet_options_tools;

SinogramCreatorMC::SinogramCreatorMC(const char* name) : SinogramCreator(name) {}

SinogramCreatorMC::~SinogramCreatorMC() {}

bool SinogramCreatorMC::init()
{
  SinogramCreatorMC::setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH2I("reconstuction_histogram", "reconstuction histogram",
                                  std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, 0.f,
                                  fMaxReconstructionLayerRadius, kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(
    new TH1F("pos_dis", "Position distance real data", (fMaxReconstructionLayerRadius) * 10 * 5, 0.f, fMaxReconstructionLayerRadius));
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

  generateSinogram();
  return true;
}

void SinogramCreatorMC::generateSinogram()
{
  std::ifstream in(fInputData);

  float firstX = 0.f;
  float firstY = 0.f;
  float secondX = 0.f;
  float secondY = 0.f;
  float firstZ = 0.f;
  float secondZ = 0.f;
  float firstT = 0.f;
  float secondT = 0.f;

  int numberOfCorrectI = 0;

  const int maxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new JPetRecoImageTools::Matrix2DProj*[fZSplitNumber];
    for (int i = 0; i < fZSplitNumber; i++) {
      fSinogram[i] = new JPetRecoImageTools::Matrix2DProj(maxDistanceNumber, (std::vector<double>(kReconstructionMaxAngle, 0)));
    }
  }

  while (in.peek() != EOF) {

    in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT;

    if (analyzeHits(firstX, firstY, firstZ, firstT,
                    secondX, secondY, secondZ, secondT)) {
      numberOfCorrectI++;
    }
  }

  std::cout << "Correct i: " << numberOfCorrectI << std::endl;
}

bool SinogramCreatorMC::exec()
{
  return true;
}

bool SinogramCreatorMC::terminate()
{

  JPetFilterNone noneFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;

  for (int i = 0; i < fZSplitNumber; i++) {
    JPetRecoImageTools::Matrix2DProj result =
      JPetRecoImageTools::backProjectWithTOF((*fSinogram[i]), fTOFInformation[i], (*fSinogram[i])[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

    saveResult(result, fOutFileName + "reconstruction_with_tof_" + std::to_string(i) + ".ppm");
    JPetRecoImageTools::Matrix2DProj filteredSinogram = JPetRecoImageTools::FilterSinogram(f, noneFilter, (*fSinogram[i]));
    JPetRecoImageTools::Matrix2DProj resultBP =
      JPetRecoImageTools::backProject(filteredSinogram, (*fSinogram[i])[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

    saveResult(resultBP, fOutFileName + "reconstruction_" + std::to_string(i) + ".ppm");
  }

  for (int i = 0; i < fZSplitNumber; i++) {
    saveResult((*fSinogram[i]), fOutFileName + "_" + std::to_string(i) + ".ppm");
  }
  delete[] fSinogram;
  delete[] fMaxValueInSinogram;

  return true;
}

void SinogramCreatorMC::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy)) {
    fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy);
  }

  if (isOptionSet(opts, kZSplitNumber)) {
    fZSplitNumber = getOptionAsInt(opts, kZSplitNumber);
  }

  if (isOptionSet(opts, kScintillatorLenght)) {
    fScintillatorLenght = getOptionAsFloat(opts, kScintillatorLenght);
  }

  if (isOptionSet(opts, kMaxReconstructionRadius)) {
    fMaxReconstructionLayerRadius = getOptionAsFloat(opts, kMaxReconstructionRadius);
  }

  if (isOptionSet(opts, kInputDataKey)) {
    fInputData = getOptionAsString(opts, kInputDataKey);
  }

  if (isOptionSet(opts, kEnableNonPerperdicualLOR)) {
    fEnableNonPerperdicularLOR = getOptionAsBool(opts, kEnableNonPerperdicualLOR);
  }

  if (isOptionSet(opts, kEnableTOFReconstrution)) {
    fEnableTOFReconstruction = getOptionAsBool(opts, kEnableTOFReconstrution);
  }

  fTOFInformation = new JPetRecoImageTools::Matrix2DTOF[fZSplitNumber];
  fMaxValueInSinogram = new int[fZSplitNumber];
  fCurrentValueInSinogram = new int[fZSplitNumber];
  const float maxZRange = fScintillatorLenght / 2.f;
  float range = (2.f * maxZRange) / fZSplitNumber;
  for (int i = 0; i < fZSplitNumber; i++) {
    float rangeStart = (i * range) - maxZRange;
    float rangeEnd = ((i + 1) * range) - maxZRange;
    fZSplitRange.push_back(std::make_pair(rangeStart, rangeEnd));
    fCurrentValueInSinogram[i] = 0;
    fMaxValueInSinogram[i] = 0;
  }

  fMaxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
}