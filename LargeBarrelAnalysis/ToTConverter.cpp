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
 *  @file ToTConverter.cpp
 */

#include <TFormula.h>
#include "ToTConverter.h"
#include "JPetLoggerInclude.h"

ToTConverter::ToTConverter(const ToTConverterParams& params): fParams(params)
{
  TFormula func("myFunc", fParams.fFormula.c_str());
  func.SetParameters(fParams.fParams.data());
  if (fParams.fBins <= 0) {
    ERROR("Number of bins must be greater than 0! getEdep() function will not work correctly");
    fParams.fValidFunction = false;
    return;
  }
  double step = (fParams.fEdepMax - fParams.fEdepMin) / fParams.fBins;
  if (step <= 0) {
    ERROR("Check values of EdepMin and EdepMax! getEdep() function will not work correctly");
    fParams.fValidFunction = false;
    return;
  }
  fValues.reserve(fParams.fBins);
  double currEdep = fParams.fEdepMin;
  for (int i = 0; i < fParams.fBins; i++) {
    fValues.push_back(func.Eval(currEdep));
    currEdep = currEdep + step;
  }
  fParams.fValidFunction = true;
}

int ToTConverter::edepToIndex(double Edep) const
{
  double step = (fParams.fEdepMax - fParams.fEdepMin) / fParams.fBins;
  assert(step > 0);
  return Edep / step; /// maybe some floor or round needed?
}

double ToTConverter::getToT(double eDep) const
{
  if ((eDep < fParams.fEdepMin) || (eDep > fParams.fEdepMax)) return 0;
  int index = edepToIndex(eDep);
  assert(index >= 0);
  assert(index < fValues.size());
  return fValues[index];
}

ToTConverterParams ToTConverter::getParams() const
{
  return fParams;
}

std::vector<double> ToTConverter::getValues() const
{
  return fValues;
}
