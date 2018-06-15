/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef UNIVERSALFILELOADER_H
#define UNIVERSALFILELOADER_H

/**
 * @file UniversalFileLoader.h
 * @brief Tools for loading any ASCII file with cnfiguration parameters
 *
 * Set of tools allowing readout of set of configuration parameters form ASCII file
 * that is in standard format of Layer-Slot-Side-Threshold
 * Contains of structure of records that has to be initialized by user,
 * and methods of reading and validating constatns.
 */

#include <map>
#include <string>
#include "JPetPM/JPetPM.h"

/**
 * POD structure, allowed values for fields (-1 corresponds to not set)
 * Layer: 1-3
 * Slot: 1-96
 * Side: JPetPM::SideA or JPetPM::SideB
 * Threshold nubmer: 1-4
 */
struct ConfRecord {
  int layer;
  int slot;
  JPetPM::Side side;
  int thresholdNumber;
  std::vector<double> parameters;
};

class UniversalFileLoader
{
public:
  typedef std::map<unsigned int, std::vector<double>> TOMBChToParameter;
  typedef std::map<std::tuple<int, int, JPetPM::Side, int>, int> TOMBChMap;
  static double getConfigurationParameter(const TOMBChToParameter& confParameters, const unsigned int channel);
  static TOMBChToParameter loadConfigurationParameters(const std::string& confFile, const TOMBChMap& tombMap);
  static TOMBChToParameter generateConfigurationParameters(const std::vector<ConfRecord>& confRecords,  const TOMBChMap& tombMap);
  static std::vector<ConfRecord> readConfigurationParametersFromFile(const std::string& confFile);
  static bool fillConfRecord(const std::string& input, ConfRecord& outRecord);
  static bool areConfRecordsValid(const std::vector<ConfRecord>& records);

private:
  UniversalFileLoader(const UniversalFileLoader&);
  void operator=(const UniversalFileLoader&);
};
#endif /*  !UNIVERSALFILELOADER_H */
