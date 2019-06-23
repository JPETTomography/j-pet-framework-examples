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
 *  @file SinogramCreatorTOF.cpp
 */

#include "SinogramCreatorTOF.h"
#include <TH1F.h>
#include <TH2I.h>
using namespace jpet_options_tools;

SinogramCreatorTOF::SinogramCreatorTOF(const char* name) : SinogramCreator(name) {}

SinogramCreatorTOF::~SinogramCreatorTOF() {}

bool SinogramCreatorTOF::init()
{
  SinogramCreatorTOF::setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH2I("reconstuction_histogram", "reconstuction histogram",
                                           std::ceil(fMaxReconstructionLayerRadius * 2 * (1.f / fReconstructionDistanceAccuracy)) + 1, 0.f,
                                           fMaxReconstructionLayerRadius, kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(
      new TH1F("pos_dis", "Position distance real data", (fMaxReconstructionLayerRadius)*10 * 5, 0.f, fMaxReconstructionLayerRadius));
  getStatistics().createHistogram(new TH1F("angle", "Position angle real data", kReconstructionMaxAngle, 0, kReconstructionMaxAngle));

  getStatistics().createHistogram(new TH1I("lor_v_slice", "Number of lors vs slice number", fZSplitNumber, -fZSplitNumber / 2., fZSplitNumber / 2.));

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
  getStatistics().getObject<TH2I>("reconstuction_histogram")->SetBit(TH2::kCanRebin);
  getStatistics().getObject<TH1F>("angle")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1F>("pos_dis")->SetBit(TH1::kCanRebin);
  getStatistics().getObject<TH1I>("lor_v_slice")->SetBit(TH1::kCanRebin);
#else
  getStatistics().getObject<TH2I>("reconstuction_histogram")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("angle")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1F>("pos_dis")->SetCanExtend(TH1::kAllAxes);
  getStatistics().getObject<TH1I>("lor_v_slice")->SetCanExtend(TH1::kAllAxes);
#endif

  generateSinogram();
  return true;
}

void SinogramCreatorTOF::generateSinogram()
{
  float firstX = 0.f;
  float firstY = 0.f;
  float secondX = 0.f;
  float secondY = 0.f;
  float firstZ = 0.f;
  float secondZ = 0.f;
  double firstT = 0.f;
  double secondT = 0.f;
  float skip = 0.f;
  int coincidence = 0;

  int numberOfCorrectHits = 0;
  int totalHits = 1; // to make sure that we do not divide by 0

  fSinogramDataTOF = JPetSinogramType::WholeSinogram(fZSplitNumber, JPetSinogramType::Matrix3D());

  for (const auto& inputPath : fInputData)
  {
    numberOfCorrectHits = 0;
    std::ifstream in(inputPath);
    while (in.peek() != EOF)
    {
      in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT >> skip >> skip >> skip >> skip >> coincidence >> skip >>
          skip >> skip;
      // in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT;
      if (coincidence != 1) // 1 == true event
        continue;

      if (analyzeHits(firstX, firstY, firstZ, firstT, secondX, secondY, secondZ, secondT))
      {
        numberOfCorrectHits++;
        // if(numberOfCorrectHits % 10000 == 0)
        //  break;
      }
      totalHits++;
    }
  }

  std::cout << "Correct hits: " << numberOfCorrectHits << " total hits: " << totalHits
            << " (correct percentage: " << (((float)numberOfCorrectHits * 100.f) / (float)totalHits) << "%)" << std::endl
            << std::endl;
}

bool SinogramCreatorTOF::exec() { return true; }

bool SinogramCreatorTOF::terminate()
{
  // Save sinogram to root file.
  JPetSinogramType map("Sinogram", fZSplitNumber);
  map.addSinogram(fSinogramDataTOF);
  JPetWriter* writer = new JPetWriter(fOutFileName.c_str());
  map.saveSinogramToFile(writer);
  writer->closeFile();

  /*
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW1D;

  for (int i = 0; i < fZSplitNumber; i++)
  {
    int sliceNumber = i - (fZSplitNumber / 2);
    if (std::find(fReconstructSliceNumbers.begin(), fReconstructSliceNumbers.end(), sliceNumber) == fReconstructSliceNumbers.end())
      continue;
    for (float value = 0.95; value <= 1.0; value += 0.01)
    {
      JPetFilterRamLak ramLakFilter(value);
      JPetSinogramType::Matrix3D filtered;
      int tofID = 0;
      for (auto& sinogram : fSinogramDataTOF[i])
      {
        filtered[sinogram.first] = JPetRecoImageTools::FilterSinogram(f, ramLakFilter, sinogram.second);
        // saveResult((filtered[sinogram.first]), fOutFileName + "sinogram_" + std::to_string(sliceNumber) + "_" +
        // std::to_string(fZSplitRange[i].first) +
        //                                          "_" + std::to_string(fZSplitRange[i].second) + "_TOFID_" + std::to_string(tofID) + ".ppm");
        tofID++;
      }
      JPetSinogramType::SparseMatrix result = JPetRecoImageTools::backProjectRealTOF(
          filtered, kReconstructionMaxAngle, JPetRecoImageTools::nonRescale, 0, 255, fReconstructionDistanceAccuracy, fTOFSliceSize,
          fTOFSigma * 2.99792458 * fReconstructionDistanceAccuracy); // TODO: change speed to light to const

      saveResult(result, fOutFileName + "reconstruction_with_TOFFBP_RamLakCutOff_" + std::to_string(value) + "_slicenumber_" +
                             std::to_string(sliceNumber) + "_" + std::to_string(fZSplitRange[i].first) + "_" +
                             std::to_string(fZSplitRange[i].second) + ".ppm");
    }
  }
  delete[] fSinogram;
  delete[] fMaxValueInSinogram;
  */
  return true;
}

bool SinogramCreatorTOF::analyzeHits(const float firstX, const float firstY, const float firstZ, const double firstTOF, const float secondX,
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
    WARNING("WARNING, reconstructed sinogram slice is out of range: " + std::to_string(i) + " max slice number: " + std::to_string(fZSplitNumber));
    return false;
  }
  const auto sinogramResult = SinogramCreatorTools::getSinogramRepresentation(
      firstX, firstY, secondX, secondY, fMaxReconstructionLayerRadius, fReconstructionDistanceAccuracy, fMaxDistanceNumber, kReconstructionMaxAngle);
  const auto TOFSlice = SinogramCreatorTools::getTOFSlice(firstTOF, secondTOF, fTOFSliceSize);
  const auto data = fSinogramDataTOF[i].find(TOFSlice);
  if (sinogramResult.first >= fMaxDistanceNumber || sinogramResult.second >= kReconstructionMaxAngle)
    return false;
  if (data != fSinogramDataTOF[i].end())
  {
    data->second(sinogramResult.first, sinogramResult.second) += 1.;
  }
  else
  {
    fSinogramDataTOF[i].insert(std::make_pair(
        TOFSlice, JPetSinogramType::SparseMatrix(fMaxDistanceNumber, kReconstructionMaxAngle, fMaxDistanceNumber * kReconstructionMaxAngle)));
    fSinogramDataTOF[i][TOFSlice](sinogramResult.first, sinogramResult.second) += 1.;
  }

  return true;
}
void SinogramCreatorTOF::setUpOptions()
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
  if (isOptionSet(opts, kMaxReconstructionRadius))
  {
    fMaxReconstructionLayerRadius = getOptionAsFloat(opts, kMaxReconstructionRadius);
  }
  if (isOptionSet(opts, kInputDataKey))
  {
    fInputData = getOptionAsVectorOfStrings(opts, kInputDataKey);
  }
  if (isOptionSet(opts, kEnableObliqueLORRemapping))
  {
    fEnableObliqueLORRemapping = getOptionAsBool(opts, kEnableObliqueLORRemapping);
  }
  if (isOptionSet(opts, kEnableTOFReconstruction))
  {
    fEnableKDEReconstruction = getOptionAsBool(opts, kEnableTOFReconstruction);
  }
  if (isOptionSet(opts, kTOFSliceSize))
  {
    fTOFSliceSize = getOptionAsFloat(opts, kTOFSliceSize);
  }
  if (isOptionSet(opts, kReconstructionTOFSigma))
  {
    fTOFSigma = getOptionAsFloat(opts, kReconstructionTOFSigma);
  }
  if (isOptionSet(opts, kRamLakCutOffValue))
  {
    fRamLakCutOffValue = getOptionAsDouble(opts, kRamLakCutOffValue);
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

  if (isOptionSet(opts, kReconstructSliceNumbers))
  {
    fReconstructSliceNumbers = boost::any_cast<std::vector<int>>(
        getOptionValue(opts, kReconstructSliceNumbers)); // TODO: Change to framework helper method when it will be accepted and merged
  }
  else
  {
    for (int i = 0; i < fZSplitNumber; i++)
    {
      fReconstructSliceNumbers.push_back(i);
    }
  }
}
