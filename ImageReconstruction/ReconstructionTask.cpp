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
  JPetSinogramType* map = JPetSinogramType::readMapFromFile("sinogramMC.root", "Sinogram");
  if (!map)
  {
    return false;
  }
  fSinogram = map->getSinogram();
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

  for (int i = 0; i < fSinogram.size(); i++)
  { // loop throught Z slices
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
    int i = 0;
    int j = 0;

    for (auto& tofWindow : sliceNumber)
    {
      // int sliceNumber = i - (fZSplitNumber / 2);
      saveResult(tofWindow.second, "test_slice_" + std::to_string(j) + "_tofwindow_" + std::to_string(i) + ".ppm");
      i++;
    }
    j++;
  }

  return true;
}

void ReconstructionTask::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kReconstructSliceNumbers))
  {
    fReconstructSliceNumbers = getOptionAsVectorOfInts(opts, kReconstructSliceNumbers));
  }
  else
  {
    for (int i = 0; i < fZSplitNumber; i++)
    {
      fReconstructSliceNumbers.push_back(i);
    }
  }
}
