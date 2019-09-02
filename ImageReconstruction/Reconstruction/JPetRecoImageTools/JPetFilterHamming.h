/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
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
 *  @file JPetFilterHamming.h
 */

#ifndef _JPetFilterHamming_H_
#define _JPetFilterHamming_H_
#include "./JPetFilterInterface.h"
#include <cmath>

/*! \brief Filter F(x) = x * (alpha + (1. - alpha * cos (2. * pi * x)))
 * default alpha = 0.54
 */
class JPetFilterHamming : public JPetFilterInterface {
public:
  JPetFilterHamming() {}
  explicit JPetFilterHamming(double maxCutOff, int size) : fCutOff(maxCutOff), fSize(size), filterValues(size) { initFilter(); }
  explicit JPetFilterHamming(double maxCutOff, double alphaValue, int size) : fAlpha(alphaValue), fCutOff(maxCutOff), fSize(size), filterValues(size)
  {
    initFilter();
  }

  virtual double operator()(int n) override { return filterValues[n]; }

  void initFilter()
  {
    float f = 0.0;
    for (int i = 1; i <= fSize - 1; i++)
    {
      f = (float)((float)0.5 * (i - 1) / fSize);
      if (f <= fCutOff)
        filterValues[i] = f * (fAlpha + (1. - fAlpha) * std::cos(M_PI * f / fCutOff));
      else
        filterValues[i] = 0.0;
    }
    if (0.5 <= fCutOff)
      filterValues[2] = (0.5) * (fAlpha + (1. - fAlpha) * std::cos(M_PI * f / fCutOff));
    else
      filterValues[2] = 0.;
  }

private:
  double fAlpha = 0.1;
  double fCutOff = 0.3;
  int fSize = 0;
  std::vector<double> filterValues;
  JPetFilterHamming(const JPetFilterHamming&) = delete;
  JPetFilterHamming& operator=(const JPetFilterHamming&) = delete;
};

#endif /*  !_JPetFilterHamming_H_ */
