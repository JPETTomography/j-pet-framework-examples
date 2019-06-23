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
 *  @file ReconstructionTask.cpp
 */

#include "ReconstructionTask.h"

using namespace jpet_options_tools;

ReconstructionTask::ReconstructionTask(const char* name) : JPetUserTask(name) {}

ReconstructionTask::~ReconstructionTask() {}

bool ReconstructionTask::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  fSinogram = JPetSinogramType::readMapFromFile("sinogramMC.root", "Sinogram");
  if (!fSinogram)
  {
    return false;
  }
  return true;
}

bool ReconstructionTask::exec() { return true; }

int ReconstructionTask::getMaxValue(const JPetSinogramType::SparseMatrix& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      if (static_cast<int>(result(i, j)) > maxValue)
        maxValue = static_cast<int>(result(i, j));
    }
  }
  return maxValue;
}

void ReconstructionTask::saveResult(const JPetSinogramType::SparseMatrix& result, const std::string& outputFileName)
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
      if (resultInt < 0)
      {
        resultInt = 0;
      }
      res << resultInt << " ";
    }
    res << std::endl;
  }
  res.close();
}

bool ReconstructionTask::terminate()
{
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW1D;
  const auto& sinogram = fSinogram->getSinogram();
  unsigned int zSplitNumber = fSinogram->getZSplitNumber();
  for (unsigned int i = 0; i < zSplitNumber; i++)
  { // loop throught Z slices
    int sliceNumber = i - (zSplitNumber / 2);
    if (!fReconstructSliceNumbers.empty()) // if there we want to reconstruct only selected z slices, skip others
      if (std::find(fReconstructSliceNumbers.begin(), fReconstructSliceNumbers.end(), sliceNumber) == fReconstructSliceNumbers.end())
        continue;
    for (float value = fCutOffValueBegin; value <= fCutOffValueEnd; value += fCutOffValueStep)
    {
      JPetFilterInterface* filter;
      static std::map<std::string, int> filterNameToFilter{{"None", ReconstructionTask::kFilterType::kFilterNone},
                                                           {"RamLak", ReconstructionTask::kFilterType::kFilterRamLak},
                                                           {"Cosine", ReconstructionTask::kFilterType::kFilterCosine},
                                                           {"Hamming", ReconstructionTask::kFilterType::kFilterHamming},
                                                           {"Hann", ReconstructionTask::kFilterType::kFilterHann},
                                                           {"Ridgelet", ReconstructionTask::kFilterType::kFilterRidgelet},
                                                           {"SheppLogan", ReconstructionTask::kFilterType::kFilterSheppLogan}};

      switch (filterNameToFilter[fFilterName])
      {
      case ReconstructionTask::kFilterType::kFilterNone:
        filter = new JPetFilterNone(value);
        break;
      case ReconstructionTask::kFilterType::kFilterRamLak:
        filter = new JPetFilterRamLak(value);
        break;
      case ReconstructionTask::kFilterType::kFilterCosine:
        filter = new JPetFilterCosine(value);
        break;
      case ReconstructionTask::kFilterType::kFilterHamming:
        filter = new JPetFilterHamming(value);
        break;
      case ReconstructionTask::kFilterType::kFilterHann:
        filter = new JPetFilterHann(value);
        break;
      case ReconstructionTask::kFilterType::kFilterRidgelet:
        filter = new JPetFilterRidgelet(value);
        break;
      case ReconstructionTask::kFilterType::kFilterSheppLogan:
        filter = new JPetFilterSheppLogan(value);
        break;
      default:
        ERROR("Could not find filter: " + fFilterName + ", using JPetFilterNone.");
        filter = new JPetFilterNone(value);
        break;
      }

      JPetSinogramType::Matrix3D filtered;
      int tofID = 0;
      for (auto& tofWindow : sinogram[i])
      {
        filtered[tofWindow.first] = JPetRecoImageTools::FilterSinogram(f, *filter, tofWindow.second);
        tofID++;
      }
      JPetSinogramType::SparseMatrix result =
          JPetRecoImageTools::backProjectRealTOF(filtered, fSinogram->getReconstructionDistanceAccuracy(), fSinogram->getTOFWindowSize(),
                                                 fSinogram->getLORTOFSigmaSize(), JPetRecoImageTools::nonRescale, 0, 255);

      saveResult(result, fOutFileName + "reconstruction_with_TOFFBP_RamLakCutOff_" + std::to_string(value) + "_slicenumber_" +
                             std::to_string(sliceNumber) + ".ppm");
    }
  }

  return true;
}

void ReconstructionTask::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey))
  {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  if (isOptionSet(opts, kReconstructSliceNumbers))
  {
    fReconstructSliceNumbers = getOptionAsVectorOfInts(opts, kReconstructSliceNumbers);
  }
}
