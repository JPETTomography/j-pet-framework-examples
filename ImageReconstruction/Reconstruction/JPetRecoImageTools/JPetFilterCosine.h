  
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
  explicit JPetFilterCosine(double maxCutOff, double) : fCutOff(maxCutOff) {}
  virtual double operator()(double radius) override { return radius < fCutOff ? radius * std::cos(M_PI * radius / fCutOff) : 0.; }

private:
  JPetFilterCosine(const JPetFilterCosine &) = delete;
  JPetFilterCosine &operator=(const JPetFilterCosine &) = delete;

  double fCutOff = 1.0f;
};

#endif /*  !_JPetFilterCosine_H_ */
