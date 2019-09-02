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
 *  @file JPetFilterCosine.h
 */

#ifndef _JPetFilterCosine_H_
#define _JPetFilterCosine_H_
#include "./JPetFilterInterface.h"
#include <cmath>

/*! \brief Filter F(x) = x * cos(x * pi)
*/
class JPetFilterCosine : public JPetFilterInterface
{
public:
  JPetFilterCosine() {}
  explicit JPetFilterCosine(double maxCutOff, int size) : fCutOff(maxCutOff), fSize(size), filterValues(size) { initFilter(); }
  virtual double operator()(int n) override { return filterValues[n]; }

  void initFilter()
  {
    float f = 0.0;
    for (int i = 1; i <= fSize - 1; i++)
    {
      f = (float)((float)0.5 * (i - 1) / fSize);
      if (f <= fCutOff)
        filterValues[i] = f * std::cos(M_PI * f / fCutOff);
      else
        filterValues[i] = 0.0;
    }
    if (0.5 <= fCutOff)
      filterValues[2] = (0.5) * std::cos(M_PI * f / fCutOff);
    else
      filterValues[2] = 0.;
  }

private:
  JPetFilterCosine(const JPetFilterCosine &) = delete;
  JPetFilterCosine &operator=(const JPetFilterCosine &) = delete;

  double fCutOff = 0.3;
  int fSize = 0;
  std::vector<double> filterValues;
};

#endif /*  !_JPetFilterCosine_H_ */
