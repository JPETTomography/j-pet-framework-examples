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
 *  @file ToTEnergyConverterFactory.h
 */

#ifndef TOTENERGYCONVERTERFACTORY_H
#define TOTENERGYCONVERTERFACTORY_H

#include "JPetOptionsTools/JPetOptionsTools.h"
#include "ToTEnergyConverter.h"
#include <TF1.h>
#include <string>

class ToTEnergyConverterFactory
{
public:
  using MyOptions = jpet_options_tools::OptsStrAny;
  using FunctionFormula = std::string;
  using FunctionParams = std::vector<double>;
  using FunctionLimits = std::pair<double, double>;
  using FuncParamsAndLimits = std::pair<FunctionFormula, std::pair<FunctionParams, FunctionLimits>>;

  ToTEnergyConverterFactory();
  ToTEnergyConverterFactory(ToTEnergyConverterFactory const&) = delete;
  ToTEnergyConverterFactory& operator=(ToTEnergyConverterFactory const&) = delete;

  void loadOptions(const MyOptions& opts);
  tot_energy_converter::ToTEnergyConverter getToTConverter() const;
  tot_energy_converter::ToTEnergyConverter getEnergyConverter() const;

private:
  const std::string kEnergy2ToTParametersParamKey = "ToTEnergyConverterFactory_Energy2ToTParameters_std::vector<double>";
  const std::string kEnergy2ToTFunctionParamKey = "ToTEnergyConverterFactory_Energy2ToTFunction_std::string";
  const std::string kEnergy2ToTFunctionLimitsParamKey = "ToTEnergyConverterFactory_Energy2ToTFunctionLimits_std::vector<double>";

  const std::string kToT2EnergyParametersParamKey = "ToTEnergyConverterFactory_ToT2EnergyParameters_std::vector<double>";
  const std::string kToT2EnergyFunctionParamKey = "ToTEnergyConverterFactory_ToT2EnergyFunction_std::string";
  const std::string kToT2EnergyFunctionLimitsParamKey = "ToTEnergyConverterFactory_ToT2EnergyFunctionLimits_std::vector<double>";

  FuncParamsAndLimits fEnergy2ToTAll;
  FuncParamsAndLimits fToT2EnergyAll;
};

#endif /*  !TOTENERGYCONVERTERFACTORY_H */
