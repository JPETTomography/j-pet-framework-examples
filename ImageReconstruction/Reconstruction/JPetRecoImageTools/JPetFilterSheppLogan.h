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
  explicit JPetFilterSheppLogan(float maxCutOff) : fCutOff(maxCutOff) {}
  virtual double operator()(double pos) override { 
    return 0.;
    /*if(n == 0) {
      return 0.02;
    }
    double ramlak = (double)(n) / (double) (m);
    if (ramlak >= fCutOff) return 0.;
    return ramlak * std::sin(2. * M_PI * ((double)(n) / ((double)(m) * M_PI * fCutOff)));*/
  }

private:
  JPetFilterSheppLogan(const JPetFilterSheppLogan&) = delete;
  JPetFilterSheppLogan& operator=(const JPetFilterSheppLogan&) = delete;
  float fCutOff = 1.f;
};

#endif /*  !_JPetFilterSheppLogan_H_ */
