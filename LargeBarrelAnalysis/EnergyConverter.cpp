/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
 *  @file EnergyConverter.cpp
 */

#include "EnergyConverter.h"
#include "JPetLoggerInclude.h"

EnergyConverter getEnergyConverter()
{
  JPetCachedFunctionParams params("[0]+ [1] * TMath::Log(x - [2]) + [3]* "
                                  "TMath::Power(TMath::Log(x - [2]), 2)",
                                  {-2332.32, 632.038, 606.909, -42.0769});
  EnergyConverter conv(params, Range(10000, 100., 940.));
  return conv;
}

EnergyConverter::EnergyConverter(const EnergyParams& params, const EnergyRange range) : fFunction(params, range) {}

double EnergyConverter::operator()(double tot) const { return fFunction(tot); }
