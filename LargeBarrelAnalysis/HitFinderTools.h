/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
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
#include "ToTEnergyConverter.h"
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
  enum TOTCalculationType
  {
    kSimplified,
    kThresholdRectangular,
    kThresholdTrapeze
  };
  static void sortByTime(std::vector<JPetPhysSignal>& signals);
  static std::map<int, std::vector<JPetPhysSignal>> getSignalsBySlot(
    const JPetTimeWindow* timeWindow, bool useCorrupts
  );
  static std::vector<JPetHit> matchAllSignals(
    std::map<int, std::vector<JPetPhysSignal>>& allSignals,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    double timeDiffAB, int refDetScinId, bool convertToT,
    const tot_energy_converter::ToTEnergyConverter& totConverter,
    JPetStatistics& stats, bool saveHistos
  );
  static std::vector<JPetHit> matchSignals(
    std::vector<JPetPhysSignal>& slotSignals,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    double timeDiffAB, bool convertToT,
    const tot_energy_converter::ToTEnergyConverter& totConverter,
    JPetStatistics& stats, bool saveHistos
  );
  static JPetHit createHit(
    const JPetPhysSignal& signal1, const JPetPhysSignal& signal2,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    bool convertToT, const tot_energy_converter::ToTEnergyConverter& totConverter,
    JPetStatistics& stats, bool saveHistos
  );
  static JPetHit createDummyRefDetHit(const JPetPhysSignal& signal);
  static int getProperChannel(const JPetPhysSignal& signal);
  static void checkTheta(const double& theta);
  static void setTOTCalculationType(std::string type);
  static double calculateTOT(const JPetHit& hit);
  static double calculateTOTside(const std::map<int, double> & thrToTOT_side);
private:
    
  static TOTCalculationType fTOTCalculationType;
};

#endif /* !HITFINDERTOOLS_H */
