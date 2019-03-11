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
  double step = (fParams.fXMax - fParams.fXMin) / fParams.fBins;
  if (step <= 0) {
    ERROR("Check values of EdepMin:" + std::to_string(fParams.fXMin) << " and EdepMax:" << std::to_string(fParams.fXMax) << " !!! getEdep() function will not work correctly.");
    fParams.fValidFunction = false;
    return;
  }
  fStep = step;
  fValues.reserve(fParams.fBins);
  double currEdep = fParams.fXMin;
  for (int i = 0; i < fParams.fBins; i++) {
    fValues.push_back(func.Eval(currEdep));
    currEdep = currEdep + step;
  }
  fParams.fValidFunction = true;
}

double ToTConverter::operator()(double x) const
{
  if ((x < fParams.fXMin) || (x > fParams.fXMax)) return 0;
  int index = edepToIndex(x);
  assert(index >= 0);
  assert(index < fValues.size());
  return fValues[index];
}

int ToTConverter::edepToIndex(double Edep) const
{
  assert(fStep > 0);
  return Edep / fStep; /// maybe some floor or round needed?
}

ToTConverterParams ToTConverter::getParams() const
{
  return fParams;
}

std::vector<double> ToTConverter::getValues() const
{
  return fValues;
}
