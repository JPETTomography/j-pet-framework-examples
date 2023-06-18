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

#include "ToTEnergyConverterFactory.h"
#include <JPetHit/JPetHit.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
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
  static std::map<int, std::vector<JPetPhysSignal>> getSignalsBySlot(const JPetTimeWindow* timeWindow, bool useCorrupts);
  static std::vector<JPetHit> matchAllSignals(std::map<int, std::vector<JPetPhysSignal>>& allSignals,
                                              const std::map<unsigned int, std::vector<double>>& velocitiesMap, double timeDiffAB, int refDetScinId,
                                              const ToTEnergyConverterFactory& totEnergyConverterFactory, JPetStatistics& stats, bool saveHistos);
  static std::vector<JPetHit> matchSignals(std::vector<JPetPhysSignal>& slotSignals, const std::map<unsigned int, std::vector<double>>& velocitiesMap,
                                           double timeDiffAB, const ToTEnergyConverterFactory& totEnergyConverterFactory, JPetStatistics& stats,
                                           bool saveHistos);
  static JPetHit createHit(const JPetPhysSignal& signal1, const JPetPhysSignal& signal2,
                           const std::map<unsigned int, std::vector<double>>& velocitiesMap,
                           const ToTEnergyConverterFactory& totEnergyConverterFactory, JPetStatistics& stats, bool saveHistos);
  static JPetHit createDummyRefDetHit(const JPetPhysSignal& signal);
  static int getProperChannel(const JPetPhysSignal& signal);
  static void checkTheta(const double& theta);
  static TOTCalculationType getTOTCalculationType(const std::string& type);
  static double calculateTOT(const JPetHit& hit, TOTCalculationType type = kSimplified);
  static double calculateTOTside(const std::map<int, double>& thrToTOT_side, TOTCalculationType type);
  static void saveTOTsync(std::vector<JPetHit>& hits, std::string TOTCalculationType, const boost::property_tree::ptree& syncTree);
  static double syncTOT(const JPetHit& hit, double TOT, const boost::property_tree::ptree& syncTree);
};

#endif /* !HITFINDERTOOLS_H */
