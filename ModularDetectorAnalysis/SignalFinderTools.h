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
 *  @file SignalFinderTools.h
 */

#ifndef SIGNALFINDERTOOLS_H
#define SIGNALFINDERTOOLS_H

/**
 * @brief Set of tools for Signal Finder task
 *
 * Contains methods building PM Signals from Signal Channels
 */

#include <boost/property_tree/ptree.hpp>

#include <JPetPMSignal/JPetPMSignal.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetSigCh/JPetSigCh.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>

#include <utility>
#include <vector>

class SignalFinderTools
{
public:
  enum TOTCalculationType
  {
    kSimplified,
    kThresholdRectangular,
    kThresholdTrapeze
  };
  static const std::map<int, std::vector<JPetSigCh>> getSigChByPM(const JPetTimeWindow* timeWindow, bool useCorruptedSigCh);
  static std::vector<JPetPMSignal> buildAllSignals(const std::map<int, std::vector<JPetSigCh>>& sigChByPM, double sigChEdgeMaxTime,
                                                   double sigChLeadTrailMaxTime, int numberOfThrs, JPetStatistics& stats, bool saveHistos,
                                                   boost::property_tree::ptree& calibTree);
  static std::vector<JPetPMSignal> buildPMSignals(const std::vector<JPetSigCh>& sigChByPM, double sigChEdgeMaxTime, double sigChLeadTrailMaxTime,
                                                  int numberOfThrs, JPetStatistics& stats, bool saveHistos, boost::property_tree::ptree& calibTree);
  static int findSigChOnNextThr(double sigChValue, double sigChEdgeMaxTime, const std::vector<JPetSigCh>& sigChVec);
  static int findTrailingSigCh(const JPetSigCh& leadingSigCh, double sigChLeadTrailMaxTime, const std::vector<JPetSigCh>& trailingSigChVec);
  static double calculatePMSignalTOT(int pmID, double totTHR1, double totTHR2, double thr1Val, double thr2Val, JPetStatistics& stats, bool saveHistos,
                                     boost::property_tree::ptree& calibTree);
};
#endif /* !SIGNALFINDERTOOLS_H */
