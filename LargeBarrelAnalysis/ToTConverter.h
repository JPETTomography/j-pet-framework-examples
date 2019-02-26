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
    fFormula(formula), fParams(params), fBins(bins), fEdepMin(min), fEdepMax(max)
  {
  }
  std::string fFormula;
  std::vector<double> fParams;
  int fBins = 100;
  double fEdepMin = -1;
  double fEdepMax = -1;
  bool fValidFunction = false;
};

class ToTConverter
{

public:
  explicit ToTConverter(const ToTConverterParams& params);

  double getToT(double eDep) const;

  int edepToIndex(double Edep) const;
  ToTConverterParams getParams() const;
  std::vector<double> getValues() const;

private:
  ToTConverterParams fParams;
  std::vector<double> fValues;
};
#endif /*  !TOTCONVERTER_H */
