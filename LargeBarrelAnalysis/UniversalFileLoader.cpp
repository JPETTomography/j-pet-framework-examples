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
 *
 *  @file UniversalFileLoader.cpp
 */

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <sstream>
#include "UniversalFileLoader.h"
#include "JPetLoggerInclude.h"

/**
 * Method returns a patameter for given TOMB channel
 */
double UniversalFileLoader::getConfigurationParameter(
  const TOMBChToParameter& confParameters,
  const unsigned int channel)
{
  if (confParameters.find(channel) == confParameters.end()) {
    DEBUG("No parameter available for the channel" + std::to_string(channel));
    return 0.0;
  } else {
    return confParameters.at(channel)[0];
  }
}

/**
 * Method loading parameters from ASCII file
 * Arguments: file name string, TOMBChMap object containing the dependency
 * between layer, barrel slot, PM side, threshold and TOMB channel number.
 */
UniversalFileLoader::TOMBChToParameter UniversalFileLoader::loadConfigurationParameters(
  const std::string& confFile,
  const UniversalFileLoader::TOMBChMap& tombMap)
{
  INFO("Loading parameters from file: " + confFile);
  if (!boost::filesystem::exists(confFile)) {
    ERROR("Configuration file does not exist: " + confFile + " Returning empty configuration.");
    TOMBChToParameter cofigurationParamteres;
    return cofigurationParamteres;
  }
  auto confRecords = readConfigurationParametersFromFile(confFile);
  return generateConfigurationParameters(confRecords, tombMap);
}

/**
 * Method generates a dependedce map between TOMB channel numbers and
 * configuration paramters. Arguments: vector of configuration records,
 * map of TOMBs with dependency between layer, barrel slot, PM side, threshold
 * and TOMB channel number.
 */
UniversalFileLoader::TOMBChToParameter UniversalFileLoader::generateConfigurationParameters(
  const std::vector<ConfRecord>& confRecords,
  const UniversalFileLoader::TOMBChMap& tombMap)
{
  TOMBChToParameter configurationParamteres;
  if (areConfRecordsValid(confRecords)) {
    std::transform(
      confRecords.begin(),
      confRecords.end(),
      std::inserter(
        configurationParamteres,
        configurationParamteres.begin()),
        [&tombMap] (const ConfRecord & confRecord) {
          auto key = std::make_tuple(confRecord.layer, confRecord.slot, confRecord.side, confRecord.thresholdNumber);
          auto parameters = confRecord.parameters;
          if (tombMap.find(key) != tombMap.end()) {
            auto tombCh = tombMap.at(key);
            return std::make_pair(tombCh, parameters);
          } else {
            ERROR("No TOMB channel number in TOMB MAP for the configuration: layer = "
              + std::to_string(confRecord.layer)
              + ", slot = " + std::to_string(confRecord.slot)
              + ", side = " + std::to_string(confRecord.side)
              + ", thresholdNumber = " + std::to_string(confRecord.thresholdNumber));
            std::vector<double> tmp = {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0};
            return std::make_pair(-1, tmp);
          }
        }
      );
    } else ERROR("Empty configuration shall be returned!");
  return configurationParamteres;
}

/**
 * Method reads the file with configuration parameters
 * and generates a vector of ConfRecords based on its content.
 * Arguments: file name string
 * In ASCII file the lines starting with # are comments and they are ignored.
 */
std::vector<ConfRecord> UniversalFileLoader::readConfigurationParametersFromFile(
  const std::string& confFile)
{
  using namespace std;
  namespace ba = boost::algorithm;

  vector<ConfRecord> confRecords;
  string inputLine;
  vector<double> inputParameters;
  ConfRecord confRecord = { -1, -1, JPetPM::SideA, -1, inputParameters};
  ifstream inputFile(confFile);
  while (getline(inputFile, inputLine)) {
    if (ba::starts_with(inputLine, "#")) continue;
    else {
      if (fillConfRecord(inputLine, confRecord)) confRecords.push_back(confRecord);
      else ERROR("Line from the configuration file seems to be incorrect:" + inputLine);
    }
  }
  return confRecords;
}

/**
 * Method validates created configuration record
 * Returs false if finds values unallowed in ConfRecord
 */
bool UniversalFileLoader::areConfRecordsValid(
  const std::vector<ConfRecord>& confRecords)
{
  const auto kMinSlot = 1;
  const auto kMaxSlot = 96;
  const auto kMinLayer = 1;
  const auto kMaxLayer = 3;
  const auto kMinThr = 1;
  const auto kMaxThr = 4;
  return ! std::any_of(
    confRecords.begin(),
    confRecords.end(),
    [](const ConfRecord & confRecord) {
      return ((confRecord.layer < kMinLayer) ||
        (confRecord.layer > kMaxLayer) ||
        (confRecord.slot < kMinSlot) ||
        (confRecord.slot > kMaxSlot) ||
        (confRecord.thresholdNumber < kMinThr) ||
        (confRecord.thresholdNumber > kMaxThr));
    }
  );
}

/**
* Method fills record parameters based on the input string.
* The input string is assumed to contain 12 elements,
* first 4 with barrel structure, another 8 with relevant parameters
* Reutrns false if the line does not conform described conditions.
*/
bool UniversalFileLoader::fillConfRecord(
  const std::string& input,
  ConfRecord& outRecord)
{
  using namespace std;
  int layer = -1;
  int slot = -1;
  char side = 'A';
  int thr = -1;
  vector<double> inputParameters = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  istringstream stream(input);
  stream >> layer >> slot >> side >> thr
  >> inputParameters.at(0) >> inputParameters.at(1) >> inputParameters.at(2)
  >> inputParameters.at(3) >> inputParameters.at(4) >> inputParameters.at(5)
  >> inputParameters.at(6) >> inputParameters.at(7);
  if (stream.fail() || (side != 'A' && side != 'B')) return false;
  else {
    outRecord.layer = layer;
    outRecord.slot = slot;
    if (side == 'A') outRecord.side = JPetPM::SideA;
    else outRecord.side = JPetPM::SideB;
    outRecord.thresholdNumber = thr;
    outRecord.parameters = inputParameters;
    return true;
  }
}
