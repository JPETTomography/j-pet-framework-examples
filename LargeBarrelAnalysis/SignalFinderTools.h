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
#include <map>
#include <array>

typedef std::array<unsigned short, 4> Permutation;
typedef std::map<unsigned int, Permutation> ThresholdOrderings;
const Permutation kIdentity = {0,1,2,3};

class SignalFinderTools
{
public:
  static const std::map<int, std::vector<JPetSigCh>> getSigChByPM(
    const JPetTimeWindow* timeWindow, bool useCorrupts
  );
  static std::vector<JPetRawSignal> buildAllSignals(
    const std::map<int, std::vector<JPetSigCh>>& sigChByPM, unsigned int numOfThresholds,
    double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
    JPetStatistics& stats, bool saveHistos,
    ThresholdOrderings thresholdOrderings
  );
  static std::vector<JPetRawSignal> buildRawSignals(
    const std::vector<JPetSigCh>& sigChByPM, unsigned int numOfThresholds,
    double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
    JPetStatistics& stats, bool saveHistos,
    Permutation ordering
  );
  static int findSigChOnNextThr(
    double sigChValue, double sigChEdgeMaxTime,
    const std::vector<JPetSigCh>& sigChVec
  );
  static int findTrailingSigCh(
    const JPetSigCh& leadingSigCh,double sigChLeadTrailMaxTime,
    const std::vector<JPetSigCh>& trailingSigChVec
  );
  static ThresholdOrderings findThresholdOrders(const JPetParamBank& bank);
};
#endif /* !SIGNALFINDERTOOLS_H */
