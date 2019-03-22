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

  getStatistics().createHistogram(new TH2I("reconstruction_histogram", "reconstruction_histogram histogram",
                                           std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, 0.f,
                                           fMaxReconstructionLayerRadius, kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(
      new TH1F("pos_dis", "Position distance real data", (fMaxReconstructionLayerRadius)*10 * 5, 0.f, fMaxReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("angle", "Position angle real data", kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
  getStatistics().getObject<TH2I>("reconstruction_histogram")->SetBit(TH2::kCanRebin);
  getStatistics().getObject<TH1F>("angle")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_dis")->SetBit(TH1::kCanRebin);
#else
  getStatistics().getObject<TH2I>("reconstruction_histogram")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("angle")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("pos_dis")->SetCanExtend(TH1::kAllAxes);
#endif
  return true;
}

bool SinogramCreator::exec()
{
  if (fSinogram == nullptr)
  {
    fSinogram = new JPetRecoImageTools::SparseMatrix*[fZSplitNumber];
    for (int i = 0; i < fZSplitNumber; i++) { fSinogram[i] = new JPetRecoImageTools::SparseMatrix(fMaxDistanceNumber, kReconstructionMaxAngle); }
  }
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      const auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      const auto hits = event.getHits();
      if (hits.size() != 2) { continue; }
      const auto& firstHit = hits[0];
      const auto& secondHit = hits[1];
      analyzeHits(firstHit, secondHit);
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

bool SinogramCreator::analyzeHits(const TVector3& firstHit, const float firstTOF, const TVector3& secondHit, const float secondTOF)
{
  int i = -1;
  if (!fEnableObliqueLORRemapping) { i = SinogramCreatorTools::getSplitRangeNumber(firstHit.Z(), secondHit.Z(), fZSplitRange); }
  else
  {
    i = SinogramCreatorTools::getSinogramSlice(firstHit.X(), firstHit.Y(), firstHit.Z(), firstTOF, secondHit.X(), secondHit.Y(), secondHit.Z(),
                                               secondTOF, fZSplitRange);
  }
  if (i < 0 || i >= fZSplitNumber)
  {
    WARNING("WARNING, reconstructed sinogram slice is out of range: " + std::to_string(i) + " max slice number: " + std::to_string(fZSplitNumber));
    return false;
  }

  const auto sinogramResult =
      SinogramCreatorTools::getSinogramRepresentation(firstHit.X(), firstHit.Y(), secondHit.X(), secondHit.Y(), fMaxReconstructionLayerRadius,
                                                      fReconstructionDistanceAccuracy, fMaxDistanceNumber, kReconstructionMaxAngle);

  (*fSinogram[i])(sinogramResult.first, sinogramResult.second) += 1.;
  fCurrentValueInSinogram[i] = (*fSinogram[i])(sinogramResult.first, sinogramResult.second);
  if (fCurrentValueInSinogram[i] > fMaxValueInSinogram[i])
  {
    fMaxValueInSinogram[i] = fCurrentValueInSinogram[i]; // save max value of sinogram
  }

  if (fEnableKDEReconstruction)
  {
    float tofRescale = 1.f;
    if (fEnableObliqueLORRemapping) { tofRescale = getTOFRescaleFactor(firstHit - secondHit); }
    auto tofInfo = fTOFInformation[i].find(sinogramResult);
    float tofResult = ((secondTOF - firstTOF) / 2.f) * tofRescale;
    if (tofInfo != fTOFInformation[i].end()) { tofInfo->second.push_back(tofResult); }
    else
    {
      fTOFInformation[i].insert(std::make_pair(sinogramResult, std::vector<float>{tofResult}));
    }
  }

  return true;
}

float SinogramCreator::getTOFRescaleFactor(const TVector3& posDiff) const
{
  float distance_3d = std::sqrt(posDiff.X() * posDiff.X() + posDiff.Y() * posDiff.Y() + posDiff.Z() * posDiff.Z());
  float distance_2d = std::sqrt(posDiff.X() * posDiff.X() + posDiff.Y() * posDiff.Y());
  if (distance_3d < kEPSILON) return 0.f;
  return distance_2d / distance_3d;
}

int SinogramCreator::getMaxValue(const JPetRecoImageTools::SparseMatrix& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      if (static_cast<int>(result(i, j)) > maxValue) maxValue = static_cast<int>(result(i, j));
    }
  }
  return maxValue;
}

void SinogramCreator::saveResult(const JPetRecoImageTools::SparseMatrix& result, const std::string& outputFileName)
{
  int maxValue = getMaxValue(result);
  std::ofstream res(outputFileName);
  res << "P2" << std::endl;
  res << result.size2() << " " << result.size1() << std::endl;
  res << maxValue << std::endl;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      int resultInt = std::round(result(i, j));
      if (resultInt < 0) { resultInt = 0; }
      res << resultInt << " ";
    }
    res << std::endl;
  }
  res.close();
}

bool SinogramCreator::terminate()
{
  for (int i = 0; i < fZSplitNumber; i++)
  {
    int sliceNumber = i - (fZSplitNumber / 2);
    if (std::find(fReconstructSliceNumbers.begin(), fReconstructSliceNumbers.end(), sliceNumber) != fReconstructSliceNumbers.end())
      saveResult((*fSinogram[i]), fOutFileName + "_" + std::to_string(sliceNumber) + ".ppm");
  }
  delete[] fSinogram;
  delete[] fMaxValueInSinogram;

  return true;
}

void SinogramCreator::setUpOptions()
{
  auto opts = getOptions();
  fOutFileName = getInputFile(opts);
  if (isOptionSet(opts, kOutFileNameKey))
  {
    fOutFileName += "_" + getOptionAsString(opts, kOutFileNameKey); // returned file name is input:
    // file name + _ + out file name + _ + slice number + .ppm
  }
  else
  {
    fOutFileName += "_sinogram";
  }

  if (isOptionSet(opts, kReconstructionDistanceAccuracy))
  { fReconstructionDistanceAccuracy = getOptionAsFloat(opts, kReconstructionDistanceAccuracy); }
  if (isOptionSet(opts, kZSplitNumber)) { fZSplitNumber = getOptionAsInt(opts, kZSplitNumber); }
  if (isOptionSet(opts, kScintillatorLenght)) { fScintillatorLenght = getOptionAsFloat(opts, kScintillatorLenght); }
  if (isOptionSet(opts, kEnableObliqueLORRemapping)) { fEnableObliqueLORRemapping = getOptionAsBool(opts, kEnableObliqueLORRemapping); }
  if (isOptionSet(opts, kEnableTOFReconstrution)) { fEnableKDEReconstruction = getOptionAsBool(opts, kEnableTOFReconstrution); }
  const JPetParamBank& bank = getParamBank();
  const JPetGeomMapping mapping(bank);
  fMaxReconstructionLayerRadius = mapping.getRadiusOfLayer(mapping.getLayersCount() - 1);

  fMaxValueInSinogram = new int[fZSplitNumber];
  fCurrentValueInSinogram = new int[fZSplitNumber];
  fTOFInformation = new JPetRecoImageTools::Matrix2DTOF[fZSplitNumber];
  const float maxZRange = fScintillatorLenght / 2.f;
  float range = (2.f * maxZRange) / fZSplitNumber;
  for (int i = 0; i < fZSplitNumber; i++)
  {
    float rangeStart = (i * range) - maxZRange;
    float rangeEnd = ((i + 1) * range) - maxZRange;
    fZSplitRange.push_back(std::make_pair(rangeStart, rangeEnd));
    fCurrentValueInSinogram[i] = 0;
    fMaxValueInSinogram[i] = 0;
  }

  fMaxDistanceNumber = std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1;

  if (isOptionSet(opts, kReconstructSliceNumbers))
  { fReconstructSliceNumbers = boost::any_cast<std::vector<int>>(getOptionValue(opts, kReconstructSliceNumbers)); }
  else
  {
    for (int i = 0; i < fZSplitNumber; i++) { fReconstructSliceNumbers.push_back(i); }
  }
}
