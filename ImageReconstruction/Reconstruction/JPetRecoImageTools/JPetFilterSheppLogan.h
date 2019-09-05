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
 *  @file JPetFilterSheppLogan.h
 */

#ifndef _JPetFilterSheppLogan_H_
#define _JPetFilterSheppLogan_H_

#include "./JPetFilterInterface.h"
#include <cmath>

/*! \brief Filter F(x) = sin(x * pi) / pi
 */
class JPetFilterSheppLogan : public JPetFilterInterface {
public:
  explicit JPetFilterSheppLogan(float maxCutOff, int size) : fCutOff(maxCutOff), fSize(size), filterValues(size) { initFilter(); }
  virtual double operator()(int n) override { return filterValues[n]; }

  void initFilter()
  {
    float f = 0.0;
    for (int i = 1; i <= fSize - 1; i++)
    {
      f = (float)((float)0.5 * (i - 1) / fSize);
      if (f <= fCutOff)
        filterValues[i] = f * std::sin(M_PI * f / fCutOff) / M_PI;
      else
        filterValues[i] = 0.0;
    }
    if (0.5 <= fCutOff)
      filterValues[2] = (0.5) * std::sin(M_PI * f / fCutOff) / M_PI;
    else
      filterValues[2] = 0.;
  }

private:
  JPetFilterSheppLogan(const JPetFilterSheppLogan&) = delete;
  JPetFilterSheppLogan& operator=(const JPetFilterSheppLogan&) = delete;
  double fCutOff = 0.3;
  int fSize = 0;
  std::vector<double> filterValues;
};

#endif /*  !_JPetFilterSheppLogan_H_ */
