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
 *  @file ToTEnergyConverter.cpp
 */

#include "ToTEnergyConverter.h"
#include "JPetLoggerInclude.h"

using namespace jpet_common_tools;
using FunctionFormula = std::string;
using FunctionLimits = std::pair<double, double>;
using FunctionParams = std::vector<double>;
using FuncParamsAndLimits = std::pair<FunctionFormula, std::pair<FunctionParams, FunctionLimits>>;
namespace tec = tot_energy_converter;

tec::ToTEnergyConverter::ToTEnergyConverter(const ToTEParams& params, const ToTERange range) : fFunction(params, range) {}

double tec::ToTEnergyConverter::operator()(double x) const { return fFunction(x); }

tec::ToTEnergyConverter generateToTEnergyConverter(const FuncParamsAndLimits& formula)
{
  auto func = formula.first;
  auto funcParams = formula.second.first;
  auto funcLimits = formula.second.second;

  JPetCachedFunctionParams params(func, funcParams);
  tec::ToTEnergyConverter conv(params, Range(100000, funcLimits.first, funcLimits.second));
  return conv;
}
