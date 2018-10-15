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
 *  @file SignalFinderTools.h
 */

#ifndef SIGNALFINDERTOOLS_H
#define SIGNALFINDERTOOLS_H

/**
 * @brief Set of tools for Signal Finder task
 *
 * Contains methods building Raw Signals from Signal Channels
 */

#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetSigCh/JPetSigCh.h>
#include <utility>
#include <vector>

class SignalFinderTools
{
public:
  static std::vector<JPetSigCh> sortByValue(std::vector<JPetSigCh> input);
  static std::vector<std::vector<JPetSigCh>> getSigChByPM(
    const JPetTimeWindow* timeWindow, const JPetParamBank& paramBank, bool useCorrupts
  );
  static std::vector<JPetRawSignal> buildAllSignals(
    std::vector<std::vector<JPetSigCh>> sigChByPM, int numOfThresholds,
    double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
    JPetStatistics& stats, bool saveHistos
  );
  static std::vector<JPetRawSignal> buildRawSignals(
    std::vector<JPetSigCh> sigChFromSamePM, int numOfThresholds,
    double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
    JPetStatistics& stats, bool saveHistos
  );
};
#endif /* !SIGNALFINDERTOOLS_H */
