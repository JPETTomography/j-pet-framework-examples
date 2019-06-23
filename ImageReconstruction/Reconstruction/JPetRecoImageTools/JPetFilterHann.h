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
 *  @file JPetFilterHann.h
 */

#ifndef _JPetFilterHann_H_
#define _JPetFilterHann_H_
#include "JPetFilterInterface.h"

/*! \brief
 */
class JPetFilterHann : public JPetFilterInterface
{
public:
  JPetFilterHann() {}
  explicit JPetFilterHann(double maxCutoff) { cutoff = maxCutoff; }
  virtual double operator()(double radius) override { return radius < cutoff ? 0.5 * (1 - std::cos(2 * M_PI * radius)) : 0.; }

private:
  double cutoff = 0.9;
};

#endif /*  !_JPetFilterHann_H_ */
