/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file Calibrate.cpp
 */

#include "CalibrationTools.h"

int main(int argc, char* argv[]) //first argument file with the parameters (default effLenParams.json)
{
  std::string fileWithParameters = "";
  if (argc == 2) {
    fileWithParameters = argv[1];
  } else if (argc < 2) {
    std::cout << "No file with calibration parameters (default - calibParams.json) as a first argument" << std::endl;
    return 0;
  } else {
    std::cout << "Too much arguments given to the program. Using only the first - file with parameters" << std::endl;  
  }
  
  EffLengthTools effLenTools(fileWithParameters);
  effLenTools.LoadCalibrationParameters();
  effLenTools.CalculateEffectiveLength();
  
  return 0;
}
