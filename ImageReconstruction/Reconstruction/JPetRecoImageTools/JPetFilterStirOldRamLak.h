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
  explicit JPetFilterStirOldRamLak(double maxCutOff, double alphaValue = 0.54) : fCutOff(maxCutOff), fAlpha(alphaValue) {}
  virtual double operator()(double n) override { return n <= fCutOff ? n * (fAlpha + (1. - fAlpha) * std::cos(M_PI * n / fCutOff)) : 0.0; }

private:
  double fAlpha = 0.54;
  double fCutOff = 1.f;
  JPetFilterStirOldRamLak(const JPetFilterStirOldRamLak&) = delete;
  JPetFilterStirOldRamLak& operator=(const JPetFilterStirOldRamLak&) = delete;
};

#endif /*  !_JPetFilterSTIROldRamLak_H_ */
