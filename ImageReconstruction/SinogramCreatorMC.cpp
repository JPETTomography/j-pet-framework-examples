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

SinogramCreatorMC::SinogramCreatorMC(const char* name) : JPetUserTask(name) {}

SinogramCreatorMC::~SinogramCreatorMC() {}

bool SinogramCreatorMC::init()
{
  setUpOptions();
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

  const int maxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;
  if (fSinogram == nullptr) {
    fSinogram = new JPetRecoImageTools::Matrix2DProj*[fZSplitNumber];
    for (int i = 0; i < fZSplitNumber; i++) {
      fSinogram[i] = new JPetRecoImageTools::Matrix2DProj(maxDistanceNumber, (std::vector<double>(kReconstructionMaxAngle, 0)));
    }
  }

  while (in.peek() != EOF) {

    in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT;

    for (int i = 0; i < fZSplitNumber; i++) {
      if (!checkSplitRange(firstZ, secondZ, i)) {
        continue;
      }
      const auto sinogramResult =
        SinogramCreatorTools::getSinogramRepresentation(firstX, firstY, secondX, secondY, fMaxReconstructionLayerRadius,
            fReconstructionDistanceAccuracy, maxDistanceNumber, kReconstructionMaxAngle);

      fCurrentValueInSinogram[i] = ++fSinogram[i]->at(sinogramResult.first).at(sinogramResult.second);
      auto tofInfo = fTOFInformation.find(sinogramResult);
      float tofResult = (secondT - firstT) / 2.f;
      if (tofInfo != fTOFInformation.end()) {
        tofInfo->second.push_back(tofResult);
      } else {
        fTOFInformation.insert(std::make_pair(sinogramResult, std::vector<float> {tofResult}));
      }
      if (fCurrentValueInSinogram[i] > fMaxValueInSinogram[i]) {
        fMaxValueInSinogram[i] = fCurrentValueInSinogram[i]; // save max value of sinogram
      }
    }
  }
}

bool SinogramCreatorMC::checkSplitRange(float firstZ, float secondZ, int i)
{
  return firstZ >= fZSplitRange[i].first && firstZ <= fZSplitRange[i].second && secondZ >= fZSplitRange[i].first && secondZ <= fZSplitRange[i].second;
}

bool SinogramCreatorMC::exec()
{
  return true;
}

int SinogramCreatorMC::getMaxValue(const JPetRecoImageTools::Matrix2DProj& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  return maxValue;
}

void SinogramCreatorMC::saveResult(const JPetRecoImageTools::Matrix2DProj& result, const std::string& outputFileName)
{
  int maxValue = getMaxValue(result);
  std::ofstream res(outputFileName);
  res << "P2" << std::endl;
  res << result[0].size() << " " << result.size() << std::endl;
  res << maxValue << std::endl;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      int resultInt = std::round(result[i][j]);
      if (resultInt < 0) {
        resultInt = 0;
      }
      res << resultInt << " ";
    }
    res << std::endl;
  }
  res.close();
}

bool SinogramCreatorMC::terminate()
{

  JPetFilterNone noneFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;

  for (int i = 0; i < fZSplitNumber; i++) {
    JPetRecoImageTools::Matrix2DProj result =
      JPetRecoImageTools::backProjectWithTOF((*fSinogram[i]), fTOFInformation, (*fSinogram[i])[0].size(), JPetRecoImageTools::nonRescale, 0, 255);
    std::string name = "reconstruction_with_tof";
    name += "_" + std::to_string(i) + ".ppm";
    saveResult(result, name);
    saveResult((*fSinogram[i]), fOutFileName + "_" + std::to_string(i) + ".ppm");
    JPetRecoImageTools::Matrix2DProj filteredSinogram = JPetRecoImageTools::FilterSinogram(f, noneFilter, (*fSinogram[i]));
    JPetRecoImageTools::Matrix2DProj resultBP =
      JPetRecoImageTools::backProject(filteredSinogram, (*fSinogram[i])[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

    name = "reconstruction";
    name += "_" + std::to_string(i) + ".ppm";
    saveResult(resultBP, name);
  }

  for (int i = 0; i < fZSplitNumber; i++) {
    std::ofstream res(fOutFileName + std::to_string(i) + ".ppm");
    res << "P2" << std::endl;
    res << (*fSinogram[i])[0].size() << " " << fSinogram[i]->size() << std::endl;
    res << fMaxValueInSinogram[i] << std::endl;
    for (unsigned int k = 0; k < fSinogram[i]->size(); k++) {
      for (unsigned int j = 0; j < (*fSinogram[i])[0].size(); j++) {
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
}