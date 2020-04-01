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
 *  @file ToTEnergyConverter.h
 */

#ifndef TOTENERGYCONVERTER_H
#define TOTENERGYCONVERTER_H

#include <string>
#include "JPetOptionsTools/JPetOptionsTools.h"

class ToTEnergyConverter
{

public:
  using MyOptions = jpet_options_tools::OptsStrAny;
  ToTEnergyConverter(ToTEnergyConverter const&) = delete;
  ToTEnergyConverter& operator=(ToTEnergyConverter const&) = delete;

  void loadOptions(const MyOptions& opts);

private:
  const std::string kEnergy2ToTParametersParamKey = "_ToTEnergyConverter_Energy2ToTParameters_std::vector<double>";
  const std::string kEnergy2ToTFunctionParamKey = "ToTEnergyConverter_Energy2ToTFunction_std::string";
  const std::string kEnergy2ToTFunctionLimitsParamKey = "ToTEnergyConverter_Energy2ToTFunctionLimits_std::vector<double>";

  const std::string kToT2EnergyParametersParamKey = "_ToTEnergyConverter_ToT2EnergyParameters_std::vector<double>";
  const std::string kToT2EnergyFunctionParamKey = "ToTEnergyConverter_ToT2EnergyFunction_std::string";
  const std::string kToT2EnergyFunctionLimitsParamKey = "ToTEnergyConverter_ToT2EnergyFunctionLimits_std::vector<double>";

};

#endif /*  !TOTENERGYCONVERTER_H */
