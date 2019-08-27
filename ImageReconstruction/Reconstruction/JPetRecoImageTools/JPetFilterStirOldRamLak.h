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

#ifndef _JPetFilterSTIROldRamLak_H_
#define _JPetFilterSTIROldRamLak_H_
#include "./JPetFilterInterface.h"
#include <cmath>
#include <iostream>

class JPetFilterStirOldRamLak : public JPetFilterInterface
{
public:
  JPetFilterStirOldRamLak() {}
  explicit JPetFilterStirOldRamLak(double maxCutOff, int size) : fCutOff(maxCutOff), fSize(size), filterValues(size) {}
  explicit JPetFilterStirOldRamLak(double maxCutOff, double alphaValue, int size) : fAlpha(alphaValue), fCutOff(maxCutOff), filterValues(size) {}
  virtual double operator()(double n) override { return filterValues[std::round(n)]; }

  void initFilter()
  {
    float f = 0.0;
    for (int i = 1; i <= fSize - 1; i += 2)
    {
      f = (float)((float)0.5 * (i - 1) / fSize);
      float nu_a = f;
      if (f <= fCutOff)
        filterValues[i] = nu_a * (fAlpha + (1. - fAlpha) * std::cos(M_PI * f / fCutOff));
      else
        filterValues[i] = 0.0;
      filterValues[i + 1] = 0.0; /* imaginary part */
    }
    if (0.5 <= fCutOff) /* see realft for an explanation:data[2]=last real */
      filterValues[2] = (0.5) * (fAlpha + (1. - fAlpha) * std::cos(M_PI * f / fCutOff));
    else
      filterValues[2] = 0.;
  }

private:
  double fAlpha = 0.54;
  double fCutOff = 0.3f;
  int fSize = 0;
  std::vector<double> filterValues;
  JPetFilterStirOldRamLak(const JPetFilterStirOldRamLak&) = delete;
  JPetFilterStirOldRamLak& operator=(const JPetFilterStirOldRamLak&) = delete;
};

#endif /*  !_JPetFilterSTIROldRamLak_H_ */