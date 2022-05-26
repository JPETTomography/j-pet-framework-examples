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

/**
 * @brief Module to find calibration constants 
 *
 * It works on three surfaces of analysis
 * First: Calibration of sides AB
 * - Working after the EventFinder files level
 * - Finding correction to side B as a sum of the edges of the TDiffBA distribution (side A reference)
 * - Creating appropriate file with AB corrections for every scintillator
 * Second: Calibration of the scintillators betweeen themselves (iterative)
 * - Working after the PALSCalibrationTask files level
 * - Finding correction to scintillator based on the difference of the PAL distributions 
 * for annihilation and deexcitation hit in a given scintillator
 * - Creating appropriate file with corrections for every scintillator
 * Third: Combining two sets of calibration files to one (AB and PAL)
 * - Working only on the files created in previous steps
 * - Creating one file with the final time corrections for a given run
 *
 */

int main(int argc, char* argv[]) //first argument file with the parameters (default calibParams.json), second calibration option (single, multi or final)
{
  std::string fileWithParameters = "";
  std::string calibrationOption = "";
  if (argc == 3) {
    fileWithParameters = argv[1];
    calibrationOption = argv[2];
  } else if (argc < 3) {
    std::cout << "No file with calibration parameters (default - calibParams.json) as a first argument and/or ";
    std::cout << "calibration option (single, multi or final) as a second argument" << std::endl;
    return 0;
  } else {
    std::cout << "Too many arguments given to the program. Using only the first two - file with parameters and calibration option" << std::endl;  
  }
  
  CalibrationTools calibTools(fileWithParameters, calibrationOption);
  calibTools.LoadCalibrationParameters();
  calibTools.Calibrate();
  
  return 0;
}
