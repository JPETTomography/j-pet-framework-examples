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
 *  @file EnergyConverter.h
 */

#ifndef ENERGYCONVERTER_H
#define ENERGYCONVERTER_H
#include <JPetCachedFunction/JPetCachedFunction.h>

class EnergyConverter
{
  using EnergyParams = jpet_common_tools::JPetCachedFunctionParams;
  using EnergyRange = jpet_common_tools::Range;
  using CachedFunction = jpet_common_tools::JPetCachedFunction1D;


public:
  EnergyConverter(const EnergyParams& params, EnergyRange);
  double operator()(double tot) const;

private:
  CachedFunction fFunction;
};

EnergyConverter generateEnergyConverter(const std::pair<std::string, std::pair<std::vector<double>, std::pair<double, double>>>& formula);

#endif /*  !ENERGYCONVERTER_H */