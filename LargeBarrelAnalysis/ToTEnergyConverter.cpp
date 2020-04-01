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
 *  @file ToTEnergyConverter.cpp
 */

#include "ToTEnergyConverter.h"
#include "JPetLoggerInclude.h"

using namespace jpet_options_tools;

void ToTEnergyConverter::loadOptions(const ToTEnergyConverter::MyOptions& opts)
{
  std::vector<double> energy2ToTParameters;
  if (isOptionSet(opts, kEnergy2ToTParametersParamKey))
  {
    energy2ToTParameters = getOptionAsVectorOfDoubles(opts, kEnergy2ToTParametersParamKey);
  }

  std::string energy2ToTFormula;
  if (isOptionSet(opts, kEnergy2ToTFunctionParamKey))
  {
    energy2ToTFormula = getOptionAsString(opts, kEnergy2ToTFunctionParamKey);
  }

  std::vector<double> energy2ToTLimits;
  if (isOptionSet(opts, kEnergy2ToTFunctionLimitsParamKey))
  {
    energy2ToTLimits = getOptionAsVectorOfDoubles(opts, kEnergy2ToTFunctionLimitsParamKey);
  }

  std::vector<double> toT2EnergyParameters;
  if (isOptionSet(opts, kToT2EnergyParametersParamKey))
  {
    toT2EnergyParameters = getOptionAsVectorOfDoubles(opts, kToT2EnergyParametersParamKey);
  }

  std::string toT2EnergyFormula;
  if (isOptionSet(opts, kToT2EnergyFunctionParamKey))
  {
    toT2EnergyFormula = getOptionAsString(opts, kToT2EnergyFunctionParamKey);
  }

  std::vector<double> toT2EnergyLimits;
  if (isOptionSet(opts, kToT2EnergyFunctionLimitsParamKey))
  {
    toT2EnergyLimits = getOptionAsVectorOfDoubles(opts, kToT2EnergyFunctionLimitsParamKey);
  }
}
