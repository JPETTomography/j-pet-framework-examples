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

class ToTConverter {
  public:
  /// Returns Time-over-threshold for given deposited energy
  /// the current parametrization is par1 + par2 * eDep 
  /// Returned value in ps, and eDep is given in keV.
  static double getToT(double eDep, double  par1 = -91958, double par2 = 19341);
  private:
    std::vector<double> fValues;


};
#endif /*  !TOTCONVERTER_H */
