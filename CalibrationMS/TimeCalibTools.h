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
 *  @file TimeCalibTools.h
 *  @brief Set of helper tools to load and apply time calibration constants.
 */

#ifndef TIMECALIBTOOLS_H
#define TIMECALIBTOOLS_H

#include <map>
#include <string>
#include "../j-pet-framework/JPetPM/JPetPM.h" /// for JPetPM::Side

/// POD helper structure that stores time calibration parameters for one element.
/// It is not initialized by default!!! User is responsible for the proper initialization.
struct TimeCalibRecord {
  int layer;  /// 1-3, -1 corresponds to not set
  int slot;   ///  1-96, -1 corresponds to not set
  JPetPM::Side side; /// SideA or SideB
  int threshold; /// 1-4, -1 corresponds to not set
  double offset_value_leading;
  double offset_uncertainty_leading; /// -1 corresponds to not set
  double offset_value_trailing;
  double offset_uncertainty_trailing; /// -1 corresponds to not set
  double quality; /// -1 to not set
};

class TimeCalibTools
{
public:
  typedef std::map<unsigned int, double> TOMBChToCorrection;
  typedef std::map<std::tuple<int, int, JPetPM::Side, int>, int> TOMBChMap;
  /// Method returns a time correction for the given tombChannel.
  static double getTimeCalibCorrection(const TOMBChToCorrection& timeCalibration, const unsigned int channel);
  /// Main method to be used to load the time calibration parameters.
  /// tombMap contains the dependency between layer, barrel slot, PM side, threshold and TOMB channel number.
  static TOMBChToCorrection loadTimeCalibration(const std::string& calibFile, const TOMBChMap& tombMap);
  /// Method generates a dependedce map between TOMB channel numbers and calibration corrections.
  /// tombMap contains the dependency between layer, barrel slot, PM side, threshold and TOMB channel number.
  /// calibRecords contains the calibration parameters.
  static TOMBChToCorrection generateTimeCalibration(const std::vector<TimeCalibRecord>& calibRecords,  const TimeCalibTools::TOMBChMap& tombMap);

  /// Method reads the calibration file and generates a vector of TimeCalibRecords based on its content.
  static std::vector<TimeCalibRecord> readCalibrationRecordsFromFile(const std::string& calibFile);

  /// Method fills record parameters based on the input string.
  /// The input string is assumed to contain 6 elements e.g. 1 1 A 2 5.0 2.0
  /// The elements correspond to layer slot side threshold offset_value offset_uncertainty.
  /// layer,slot, threshold are treated as int. offset_value and offset_uncertainty are treated as double.
  /// side can have value: A or B.
  /// If the line does not conform to described condition false is return, and the record is not changed
  static bool fillTimeCalibRecord(const std::string& input, TimeCalibRecord& outRecord);
  /// Method checks if the calibration records are correct:
  /// 1. layer elements correspond to value: 1 to 3
  /// 2. slot elements correspond to values: 1 to 96
  /// 3. thresholds elements correspond to values: 1-4
  /// If any of the elements does not fullfill any of the conditions false is returned.
  static bool areCorrectTimeCalibRecords(const std::vector<TimeCalibRecord>& records);
private:
  TimeCalibTools(const TimeCalibTools&);
  void operator=(const TimeCalibTools&);
};
#endif /*  !TIMECALIBTOOLS_H */
