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
 *  @file ToTConverter.h
 */

#ifndef TOTCONVERTER_H
#define TOTCONVERTER_H

#include <vector>
#include <string>

struct ToTConverterParams {
  ToTConverterParams(const std::string& formula, const std::vector<double>& params, int bins, double min, double max):
    fFormula(formula), fParams(params), fBins(bins), fXMin(min), fXMax(max)
  {
  }
  std::string fFormula;  /// Function formula that must be understood by TFormula from ROOT.
  std::vector<double> fParams; /// Parameters used by the function described by TFormula.
  int fBins = 100;  /// Number of times the function is sampled.
  double fXMin = -1;
  double fXMax = -1;
  bool fValidFunction = false;
};

class ToTConverter
{

public:
  explicit ToTConverter(const ToTConverterParams& params);

  double operator()(double x) const;

  int edepToIndex(double Edep) const;
  ToTConverterParams getParams() const;
  std::vector<double> getValues() const;

private:
  ToTConverterParams fParams;  /// Parameters describing the function.
  std::vector<double> fValues; /// Lookup table containg the function values.
  double fStep = -1; /// Step size with which the lookup table is filled.
};
#endif /*  !TOTCONVERTER_H */
