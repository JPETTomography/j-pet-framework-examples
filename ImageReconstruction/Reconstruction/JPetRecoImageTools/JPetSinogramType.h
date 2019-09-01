/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file JPetSinogramType.h
 */

#ifndef _JPET_SinogramType_H_
#define _JPET_SinogramType_H_

#include "JPetLoggerInclude.h"
#include "JPetWriter/JPetWriter.h"

#include <boost/serialization/array_wrapper.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class JPetSinogramType : public TObject
{
public:
  using SparseMatrix = boost::numeric::ublas::mapped_matrix<double>;
  using Matrix3D = std::unordered_map<int, SparseMatrix>;
  using WholeSinogram = std::vector<Matrix3D>; // slice number, tof window, sinogram matrix

  JPetSinogramType() : fName("test") {}
  explicit JPetSinogramType(std::string name, unsigned int zSplitNumber, unsigned int maxDistanceNumber, float maxReconstructionLayerRadius,
                            float reconstructionDistanceAccuracy, float scintillatorLenght, float TOFWindowSize,
                            std::vector<std::pair<float, float>> zSplitRange)
      : fName(name), fSinogramType(zSplitNumber, Matrix3D()), fZSplitNumber(zSplitNumber), fMaxDistanceNumber(maxDistanceNumber),
        fMaxReconstructionLayerRadius(maxReconstructionLayerRadius), fReconstructionDistanceAccuracy(reconstructionDistanceAccuracy),
        fScintillatorLenght(scintillatorLenght), fTOFWindowSize(TOFWindowSize), fZSplitRange(zSplitRange)
  {
  }

  ~JPetSinogramType();

  void saveSinogramToFile(JPetWriter* writer) const
  {
    assert(writer);
    if (!writer->isOpen())
    {
      ERROR("Could not write SinogramType(" + fName + ") to file. The provided JPetWriter is closed.");
      return;
    }
    writer->writeObject(this, fName.c_str());
    return;
  }

  /* @brief Warning! This method to not check if returned variable is valid.
   *
   *
   */
  static JPetSinogramType* readMapFromFile(const std::string& filename, const std::string& name)
  {
    TFile file(filename.c_str(), "READ");
    if (!file.IsOpen())
    {
      ERROR("Could not read from file.");
      return nullptr;
    }
    JPetSinogramType* map = static_cast<JPetSinogramType*>(file.Get(name.c_str()));
    return map;
  }

  void addSlice(const SparseMatrix object, const int sliceNumber,
                const int tofWindow = 0) // copy object to make sure we do not assign temporary object
  {
    fSinogramType[sliceNumber].insert({tofWindow, object});
  }

  void addSinogram(const WholeSinogram object) // copy object to make sure we do not assign temporary object
  {
    fSinogramType = object;
  }

  WholeSinogram& getSinogram() // this function can change state of sinogram
  {
    return fSinogramType;
  }

  void setNumberOfAllEvents(unsigned int numberOfAllEvents) { fNumberOfAllEvents = numberOfAllEvents; }

  void setNumberOfEventsUsedToCreateSinogram(unsigned int numberOfEventsUsedToCreateSinogram)
  {
    fNumberOfEventsUsedToCreateSinogram = numberOfEventsUsedToCreateSinogram;
  }

  unsigned int getZSplitNumber() const { return fZSplitNumber; }
  unsigned int getMaxDistanceNumber() const { return fMaxDistanceNumber; }
  unsigned int getNumberOfAllEvents() const { return fNumberOfAllEvents; }
  unsigned int getNumberOfEventsUsedToCreateSinogram() const { return fNumberOfEventsUsedToCreateSinogram; }
  float getMaxReconstructionLayerRadius() const { return fMaxReconstructionLayerRadius; }
  float getReconstructionDistanceAccuracy() const { return fReconstructionDistanceAccuracy; }
  float getScintillatorLenght() const { return fScintillatorLenght; }
  float getTOFWindowSize() const { return fTOFWindowSize; }
  std::vector<std::pair<float, float>> getZSplitRange() const { return fZSplitRange; }

  ClassDef(JPetSinogramType, 4);

private:
  std::string fName;           // name to save in root file.
  WholeSinogram fSinogramType; // for passing any type of variables between tasks and save them to root file.

  unsigned int fZSplitNumber;
  unsigned int fMaxDistanceNumber;
  unsigned int fNumberOfAllEvents;
  unsigned int fNumberOfEventsUsedToCreateSinogram;
  float fMaxReconstructionLayerRadius;
  float fReconstructionDistanceAccuracy;
  float fScintillatorLenght;
  float fTOFWindowSize;
  std::vector<std::pair<float, float>> fZSplitRange;
};

#endif /* !_JPET_SinogramType_H_ */
