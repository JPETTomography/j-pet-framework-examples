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
#include <unordered_map>

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <string>

class JPetSinogramType : public TObject
{
public:
  using SparseMatrix = boost::numeric::ublas::mapped_matrix<double>;
  using Matrix3D = std::unordered_map<int, SparseMatrix>; 
  using WholeSinogram = std::vector<Matrix3D>; // slice number, tof window, sinogram matrix

  JPetSinogramType() : fName("test") { }
  explicit JPetSinogramType(std::string name, unsigned int size) : fName(name), fSinogramType(size, Matrix3D()) {}
  ~JPetSinogramType();

  void saveSinogramToFile(JPetWriter* writer)
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
  static JPetSinogramType* readMapFromFile(std::string filename, std::string name)
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

  void addSlice(SparseMatrix object, int sliceNumber, int tofWindow = 0) 
  { 
    fSinogramType[sliceNumber].insert({tofWindow, object}); 
  }

  void addSinogram(WholeSinogram object) { fSinogramType = object; }
  
  WholeSinogram getSinogram()
  {
    return fSinogramType;
  }

  ClassDef(JPetSinogramType, 3);

private:
  std::string fName;           // name to save in root file.
  WholeSinogram fSinogramType; // for passing any type of variables between tasks and save them to root file.
};

#endif /* !_JPET_SinogramType_H_ */
