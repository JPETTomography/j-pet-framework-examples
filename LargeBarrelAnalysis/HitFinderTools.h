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
 *  @file HitFinderTools.h
 */

#ifndef HITFINDERTOOLS_H
#define HITFINDERTOOLS_H

#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetHit/JPetHit.h>
#include <vector>

/**
 * @brief Tools set fot HitFinder module
 *
 * Tols include methods of signal mapping and matching,
 * helpers of sorting, radian check and methods for reference detecctor
 *
 */
class HitFinderTools
{
public:
  static std::map<int, std::vector<JPetPhysSignal>> getSignalsSlotMap(
    const JPetTimeWindow* timeWindow);
  static std::vector<JPetHit> matchSignals(
    JPetStatistics& stats,
    const std::map<int, std::vector<JPetPhysSignal>>& signalSlotMap,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    double timeDiffAB,
    int refDetScinId,
    bool saveHistos
  );
  static JPetHit createHit(
    const JPetPhysSignal& signalA,
    const JPetPhysSignal& signalB,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap);
  static JPetHit createDummyRefDetHit(const JPetPhysSignal& signalB);
  static void checkTheta(const double& theta);
  static int getProperChannel(const JPetPhysSignal& signal);
  static void sortByTime(std::vector<JPetPhysSignal>& side);
};

#endif /*  !HITFINDERTOOLS_H */
