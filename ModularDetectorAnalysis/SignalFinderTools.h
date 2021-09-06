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

#include <JPetParamBank/JPetParamBank.h>
#include <JPetStatistics/JPetStatistics.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <Signals/JPetChannelSignal/JPetChannelSignal.h>
#include <Signals/JPetPMSignal/JPetPMSignal.h>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include <vector>

class SignalFinderTools
{
public:
  enum ToTCalculationType
  {
    kSimplified,
    kThresholdRectangular,
    kThresholdTrapeze
  };
  static const std::map<int, std::vector<JPetChannelSignal>> getChannelSignalsByPM(const JPetTimeWindow* timeWindow, bool useCorruptedSigCh);

  static std::vector<JPetPMSignal> buildAllSignals(const std::map<int, std::vector<JPetChannelSignal>>& chSigByPM, double chSigEdgeMaxTime,
                                                   double chSigLeadTrailMaxTime, int numberOfThrs, JPetStatistics& stats, bool saveHistos,
                                                   SignalFinderTools::ToTCalculationType type, boost::property_tree::ptree& calibTree);

  static std::vector<JPetPMSignal> buildPMSignals(const std::vector<JPetChannelSignal>& chSigByPM, double chSigEdgeMaxTime,
                                                  double chSigLeadTrailMaxTime, int numberOfThrs, JPetStatistics& stats, bool saveHistos,
                                                  SignalFinderTools::ToTCalculationType type, boost::property_tree::ptree& calibTree);

  static int findChannelSignalOnNextThr(double chSigValue, double chSigEdgeMaxTime, const std::vector<JPetChannelSignal>& chSigVec);

  static int findTrailingChannelSignal(const JPetChannelSignal& leadingSigCh, double chSigLeadTrailMaxTime,
                                       const std::vector<JPetChannelSignal>& trailingSigChVec);

  static double calculatePMSignalToT(JPetPMSignal& pmSignal, SignalFinderTools::ToTCalculationType type, boost::property_tree::ptree& calibTree);
};

#endif /* !SIGNALFINDERTOOLS_H */
