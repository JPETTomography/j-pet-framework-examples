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
 *  @file TimeCalibLoader.h
 */

#ifndef TIMECALIBLOADER_H
#define TIMECALIBLOADER_H

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif

#include <JPetUserTask/JPetUserTask.h>
#include <map>

/**
 * @brief module to apply the time calibration in J-PET. It takes
 * as an input a tree of uncalibrated JPetSigCh objects and saves a tree of calibrated
 * onces.
 * The name of the file with the calibration constants should be defined
 * by user option (in in json file)  in the following format:
 * "TimeCalibLoader_ConfigFile":"path_and_filename_with_calib_constants"
 * The default file name "timeCalib.txt" will be used if the user option
 * is not set.
 * The current correction has a following formula: raw_time + 1000 * correction_constant
 * 1000 factor is needed because current calib constants are expressed in ns, while
 * JPetSigCh time is in ps.
 * If a calibration constant is missing for a given channel, then the 0 is returned.
 * Also, info to log will be sent if DEBUG level is activated.
 * The calibration is applied based on the TOMB identifier.
 *
 */
class TimeCalibLoader : public JPetUserTask
{
public:
  TimeCalibLoader(const char* name);
  virtual ~TimeCalibLoader();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;
protected:
  const std::string fConfigFileParamKey = "TimeCalibLoader_ConfigFile_std::string";  ///Name of the option for which the value would correspond to the time calibration file name.
  JPetParamManager* fParamManager = nullptr;
  std::map<unsigned int, double> fTimeCalibration;
};
#endif /*  !TIMECALIBLOADER_H */
