/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
 *  @file TimeWindowCreatorTools.h
 */

#ifndef TIMEWINDOWCREATORTOOLS_H
#define TIMEWINDOWCREATORTOOLS_H

#include <JPetTOMBChannel/JPetTOMBChannel.h>
#include <Unpacker2/Unpacker2/TDCChannel.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetSigCh/JPetSigCh.h>
#include <vector>

/**
* @brief Set of tools for Time Window Creator task
*
* Contains methods building Signals Channels from Unpacker eventsIII
*/

class TimeWindowCreatorTools
{
public:
  static std::vector<JPetSigCh> sortByValue(std::vector<JPetSigCh> input);
  static std::vector<JPetSigCh> buildSigChs(
    TDCChannel* tdcChannel, JPetTOMBChannel tombChannel, double maxTime, double minTime,
    std::map<unsigned int, std::vector<double>> timeCalibrationMap,
    std::map<unsigned int, std::vector<double>> thresholdsMap,
    bool setTHRValuesFromChannels, JPetStatistics& stats, bool saveHistos
  );
  static std::vector<JPetSigCh> flagSigChs(
    std::vector<JPetSigCh> inputSigChs, const JPetParamBank& paramBank,
    int numOfThresholds, JPetStatistics& stats, bool saveHistos
  );
  static JPetSigCh generateSigCh(
    double tdcChannelTime, const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge,
    std::map<unsigned int, std::vector<double>> timeCalibrationMap,
    std::map<unsigned int, std::vector<double>> thresholdsMap,
    bool setTHRValuesFromChannels
  );
  static std::vector<std::vector<std::vector<JPetSigCh>>> sigChByPMTHR(
    std::vector<JPetSigCh> inputSigChs, const JPetParamBank& paramBank, int numOfThresholds
  );
  static std::vector<JPetSigCh> flagTHRSigChs(
    std::vector<JPetSigCh> thrSigCh, JPetStatistics& stats, bool saveHistos
  );
};

#endif /* !TIMEWINDOWCREATORTOOLS_H */
