/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RedModuleHitFinderTools.h
 */

#ifndef REDMODULEHITFINDERTOOLS_H
#define REDMODULEHITFINDERTOOLS_H

#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <Signals/JPetMatrixSignal/JPetMatrixSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <vector>

/**
 * @brief Tools set fot RedModuleHitFinder module
 *
 * Tols include methods of signal mapping and matching, helpers of sorting
 */
class RedModuleHitFinderTools
{
public:
  static std::vector<JPetPhysRecoHit> matchHitsWithWLSSignals(const std::vector<JPetPhysRecoHit>& scinHits,
                                                              const std::map<int, std::vector<JPetMatrixSignal>>& wlsSignals, double maxTimeDiffWLS,
                                                              double timeDiffOffset, boost::property_tree::ptree& calibTree,
                                                              boost::property_tree::ptree& wlsConfig, JPetStatistics& stats, bool saveHistos);

  static JPetPhysRecoHit createWLSHit(const JPetPhysRecoHit& scinHit, const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& calibTree,
                                      boost::property_tree::ptree& wlsConfig, JPetStatistics& stats, bool saveHistos);

  static double estimateZPosWithWLS(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig);
  static double zPosMethod0(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig);
  static double zPosMethod1(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig);
  static double zPosMethod2(const JPetMatrixSignal& wlsSignal, boost::property_tree::ptree& wlsConfig);
};

#endif /* !REDMODULEHITFINDERTOOLS_H */
