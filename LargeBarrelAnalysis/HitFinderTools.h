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
  static void sortByTime(std::vector<JPetPhysSignal>& signals);
  static std::map<int, std::vector<JPetPhysSignal>> getSignalsBySlot(
    const JPetTimeWindow* timeWindow, bool useCorrupts
  );
  static std::vector<JPetHit> matchAllSignals(
    std::map<int, std::vector<JPetPhysSignal>>& allSignals,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    double timeDiffAB, int refDetScinId, JPetStatistics& stats, bool saveHistos
  );
  static std::vector<JPetHit> matchSignals(
    std::vector<JPetPhysSignal>& slotSignals,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    double timeDiffAB, JPetStatistics& stats, bool saveHistos
  );
  static JPetHit createHit(
    const JPetPhysSignal& signal1, const JPetPhysSignal& signal2,
    const std::map<unsigned int, std::vector<double>>& velocitiesMap,
    JPetStatistics& stats, bool saveHistos
  );
  static JPetHit createDummyRefDetHit(const JPetPhysSignal& signal);
  static int getProperChannel(const JPetPhysSignal& signal);
<<<<<<< b76e8c289e91932a2e7bf30b19ab6fcaaa8d2d1e
  static void checkTheta(const double& theta);
  static double calculateTOT(const JPetHit& hit);
=======
  static void sortByTime(std::vector<JPetPhysSignal>& side);
  /// Returns Time-over-threshold for given deposited energy
  /// the current parametrization is par1 + par2 * eDep 
  /// Returned value in ps, and eDep is given in keV.
  static double getToT(double eDep, double  par1 = -91958, double par2 = 19341);

>>>>>>> Add Energy to Tot function to HitFinderTools
};

#endif /* !HITFINDERTOOLS_H */
