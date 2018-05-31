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
 *  @file SignalFinderTools.h
 */

#ifndef SIGNALFINDERTOOLS_H
#define SIGNALFINDERTOOLS_H

/**
 * @brief Set of tools for Signal Finder task
 *
 * Contains methods building Raw Signals from Signal Channels
 * Core algorithm descibed in
 * http://koza.if.uj.edu.pl/petwiki/images/4/4f/SignalFinderDoc.pdf
 */

#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetSigCh/JPetSigCh.h>
#include <vector>
#include <map>

class SignalFinderTools
{
public:
  static std::map<int, std::vector<JPetSigCh>> getSigChsPMMapById(const JPetTimeWindow* timeWindow);
  static std::vector<JPetRawSignal> buildAllSignals(
    std::map<int, std::vector<JPetSigCh>> sigChsPMMap,
    unsigned int numOfThresholds,
    JPetStatistics& stats,
    double sigChEdgeMaxTime,
    double sigChLeadTrailMaxTime,
    bool saveHistos
  );

  static std::vector<JPetRawSignal> buildRawSignals(
    const std::vector<JPetSigCh>& sigChFromSamePM,
    unsigned int numOfThresholds,
    JPetStatistics& stats,
    double sigChEdgeMaxTime,
    double sigChLeadTrailMaxTime,
    bool saveHistos
  );

  static int findSigChOnNextThr(
    double sigChValue,
    const std::vector<JPetSigCh>& sigChVec,
    double sigChEdgeMaxTime
  );

  static int findTrailingSigCh(
    const JPetSigCh& leadingSigCh,
    const std::vector<JPetSigCh>& trailingSigChVec,
    double sigChLeadTrailMaxTime);

};
#endif /* !SIGNALFINDERTOOLS_H */
