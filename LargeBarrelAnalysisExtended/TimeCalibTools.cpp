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

#include <boost/algorithm/string/predicate.hpp> /// for starts_with
#include <sstream>
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
  auto calibRecords = readCalibrationRecordsFromFile(calibFile);
  /// Fake transform for a moment.
  for (const auto & record : calibRecords) {
    timeCalibration.insert(std::make_pair(record.slot, record.offset_value));
  }
  return timeCalibration;
}

std::vector<TimeCalibRecord> TimeCalibTools::readCalibrationRecordsFromFile(const std::string& calibFile)
{
  using namespace std;
  namespace ba = boost::algorithm;

  std::vector<TimeCalibRecord> timeCalibRecords;
  string line;
  TimeCalibRecord record = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  ifstream inputFile(calibFile);

  while (getline(inputFile, line)) {
    /// Lines starting with # are comments and they are ignored.
    if (ba::starts_with(line, "#")) {
      continue;
    } else {
      if (fillTimeCalibRecord(line, record)) {
        timeCalibRecords.push_back(record);
      } else {
        ERROR("Line from the time calibration file seems to be incorrect:" + line);
      }
    }
  }
  return timeCalibRecords;
}

bool TimeCalibTools::fillTimeCalibRecord(const std::string& input, TimeCalibRecord& outRecord)
{
  using namespace std;
  int layer = -1;
  int slot = -1;
  char side = 'A';
  int thr = -1;
  double o_val = 0;
  double o_uncert = 0;
  istringstream ss(input);
  ss >> layer >> slot >> side >> thr >> o_val >> o_uncert;
  if (ss.fail() || (side != 'A' && side != 'B')) {
    return false;
  } else {
    outRecord.layer = layer;
    outRecord.slot = slot;
    if (side == 'A') {
      outRecord.side = JPetPM::SideA;
    } else {
      outRecord.side = JPetPM::SideB;
    }
    outRecord.threshold = thr;
    outRecord.offset_value = o_val;
    outRecord.offset_uncertainty = o_uncert;
    return true;
  }
}
