/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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

#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <Signals/JPetMatrixSignal/JPetMatrixSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <vector>

/**
 * @brief Tools set fot HitFinder module
 *
 * Tols include methods of signal mapping and matching, helpers of sorting
 */
class HitFinderTools
{
public:
  static void sortByTime(std::vector<JPetMatrixSignal>& signals);

  static void sortByTime(std::vector<JPetPhysRecoHit>& hits);

  static std::map<int, std::vector<JPetMatrixSignal>> getSignalsByScin(const JPetTimeWindow* timeWindow);

  static std::vector<JPetPhysRecoHit> matchAllSignals(std::map<int, std::vector<JPetMatrixSignal>>& allSignals, double timeDiffAB,
                                                      boost::property_tree::ptree& calibTree, JPetStatistics& stats, bool saveHistos);

  static std::vector<JPetPhysRecoHit> matchSignals(std::vector<JPetMatrixSignal>& scinSignals, double timeDiffAB,
                                                   boost::property_tree::ptree& calibTree, JPetStatistics& stats, bool saveHistos);

  static JPetPhysRecoHit createHit(const JPetMatrixSignal& signal1, const JPetMatrixSignal& signal2, boost::property_tree::ptree& calibTree);

  static JPetPhysRecoHit createDummyHit(const JPetMatrixSignal& signal);
};

#endif /* !HITFINDERTOOLS_H */
