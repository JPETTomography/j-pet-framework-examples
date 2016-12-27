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
 *  @file TimeCalibTools.cpp
 */

#include <boost/property_tree/json_parser.hpp>

#include "TimeCalibTools.h"
#include "JPetLoggerInclude.h"

double TimeCalibTools::getTimeCalibCorrection(const TOMBChToCorrection& timeCalibration, const unsigned int channel)
{
  if (timeCalibration.find(channel) == timeCalibration.end()) {
    WARNING("No time calibration available for this channel");
    return 0.0;
  } else {
    return timeCalibration.at(channel);
  }
}

TimeCalibTools::TOMBChToCorrection TimeCalibTools::loadTimeCalibration(const std::string& calibFile)
{
  TOMBChToCorrection timeCalibration;
  using boost::property_tree::ptree;
  try {
    boost::property_tree::ptree propTree;
    read_json(calibFile, propTree);
    for (auto & corr : propTree.get_child("time_calibration")) {
    }
    /// for tests
    unsigned int channel = 1;
    double correction = 1;
    timeCalibration.insert(std::make_pair(channel, correction));
  } catch (const std::runtime_error& error) {
    std::string message = "Error opening time calibration file. Error = " + std::string(error.what());
    ERROR(message);
  }
  return timeCalibration;
}
