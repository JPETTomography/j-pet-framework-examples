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

#include "ToTConverter.h"
#include "JPetLoggerInclude.h"

using namespace jpet_common_tools;

ToTConverter getToTConverter()
{
  JPetCachedFunctionParams params("[0]+ [1] * TMath::Log(x - [2]) + [3]* "
                                  "TMath::Power(TMath::Log(x - [2]), 2)",
                                  {-2332.32, 632.038, 606.909, -42.0769});
  ToTConverter conv(params, Range(10000, 100., 940.));
  return conv;
}

ToTConverter::ToTConverter(const ToTParams& params, const ToTRange range) : fFunction(params, range) {}

double ToTConverter::operator()(double E) const { return fFunction(E); }
