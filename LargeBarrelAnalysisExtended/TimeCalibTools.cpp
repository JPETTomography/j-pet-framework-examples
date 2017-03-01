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
#include <boost/filesystem.hpp> /// for exists()
#include <algorithm> /// for any_of()
#include <sstream>
#include "TimeCalibTools.h"
#include "JPetLoggerInclude.h"

double TimeCalibTools::getTimeCalibCorrection(const TOMBChToCorrection& timeCalibration, const unsigned int channel)
{
  if (timeCalibration.find(channel) == timeCalibration.end()) {
    DEBUG("No time calibration available for the channel" + std::to_string(channel));
    return 0.0;
  } else {
    return timeCalibration.at(channel);
  }
}

TimeCalibTools::TOMBChToCorrection TimeCalibTools::loadTimeCalibration(const std::string& calibFile, const TimeCalibTools::TOMBChMap& tombMap)
{
  INFO("Loading time calibration from:" + calibFile);
  if ( !boost::filesystem::exists(calibFile)) {
    ERROR("Calibration file does not exist:" + calibFile + " Returning empty timeCalibration");
    TOMBChToCorrection timeCalibration;
    return timeCalibration;
  }
  auto calibRecords = readCalibrationRecordsFromFile(calibFile);
  return generateTimeCalibration(calibRecords, tombMap);
}

TimeCalibTools::TOMBChToCorrection TimeCalibTools::generateTimeCalibration(const std::vector<TimeCalibRecord>& calibRecords,  const TimeCalibTools::TOMBChMap& tombMap)
{
  TOMBChToCorrection timeCalibration;
  if (areCorrectTimeCalibRecords(calibRecords)) {
    std::transform(calibRecords.begin(),
                   calibRecords.end(),
                   std::inserter(timeCalibration, timeCalibration.begin()),
    [&tombMap](const TimeCalibRecord & r) {
      auto key = std::make_tuple(r.layer, r.slot, r.side, r.threshold);
      auto correction = r.offset_value_leading;
      if (tombMap.find(key) != tombMap.end()) {
        auto tombCh = tombMap.at(key);
        return std::make_pair(tombCh, correction);
      } else {
        ERROR("No TOMB channel number in TOMB MAP for the combination: layer=" + std::to_string(r.layer) + ",slot=" + std::to_string(r.slot) + ",side=" + std::to_string(r.side) + ",threshold=" + std::to_string(r.threshold));
        return std::make_pair(-1, 0.0);
      }
    });
  } else {
    ERROR("Empty calibration will be returned");
  }
  return timeCalibration;
}

std::vector<TimeCalibRecord> TimeCalibTools::readCalibrationRecordsFromFile(const std::string& calibFile)
{
  using namespace std;
  namespace ba = boost::algorithm;

  std::vector<TimeCalibRecord> timeCalibRecords;
  string line;
  TimeCalibRecord record = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1.0 };
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

bool TimeCalibTools::areCorrectTimeCalibRecords(const std::vector<TimeCalibRecord>& records)
{
  const auto kMinSlot = 1;
  const auto kMaxSlot = 96;
  const auto kMinLayer = 1;
  const auto kMaxLayer = 3;
  const auto kMinThr = 1;
  const auto kMaxThr = 4;
  return ! std::any_of(
           records.begin(),
           records.end(),
  [](const TimeCalibRecord & r) {
    return ((r.layer < kMinLayer) ||
            (r.layer > kMaxLayer) ||
            (r.slot < kMinSlot) ||
            (r.slot > kMaxSlot) ||
            (r.threshold < kMinThr) ||
            (r.threshold > kMaxThr)
           );
  });
}

bool TimeCalibTools::fillTimeCalibRecord(const std::string& input, TimeCalibRecord& outRecord)
{
  using namespace std;
  int layer = -1;
  int slot = -1;
  char side = 'A';
  int thr = -1;
  double o_val_lead = 0;
  double o_uncert_lead = 0;
  double o_val_trail = 0;
  double o_uncert_trail = 0;
  double quality = 0;
  istringstream ss(input);
  ss >> layer >> slot >> side >> thr >> o_val_lead >> o_uncert_lead >> o_val_trail >> o_uncert_trail >> quality;
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
    outRecord.offset_value_leading = o_val_lead;
    outRecord.offset_uncertainty_leading = o_uncert_lead;
    outRecord.offset_value_trailing = o_val_trail;
    outRecord.offset_uncertainty_trailing = o_uncert_trail;
    outRecord.quality = quality;
    return true;
  }
}
