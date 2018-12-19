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
 *  @file EventCategorizerTools.h
 */

#ifndef EVENTCATEGORIZERTOOLS_H
#define EVENTCATEGORIZERTOOLS_H

#include <JPetStatistics/JPetStatistics.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>

static const double kLightVelocity_cm_ns = 29.9792458;
static const double kUndefinedValue = 999.0;

/**
 * @brief Tools for Event Categorization
 *
 * Lots of tools in constatnt developement.
*/
class EventCategorizerTools
{
public:
  static bool checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats,
                             bool saveHistos, double b2bSlotThetaDiff);
  static bool checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos);
  static bool checkForPrompt(const JPetEvent& event, JPetStatistics& stats,
                             bool saveHistos, double deexTOTCutMin, double deexTOTCutMax);
  static bool checkForScatter(const JPetEvent& event, JPetStatistics& stats,
                              bool saveHistos, double scatterTOFTimeDiff);
  static double calculateTOT(const JPetHit& hit);
  static double calculateDistance(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2);
  /// Tof calculated with the ordered hits with respect to scintillator number
  static double calculateTOFArrangedByScintilatorNr(const JPetHit& hitA, const JPetHit& hitB);
  /// Tof is calculated as  time1 -time2. The order is random
  static double calculateTOF(const JPetHit& hitA, const JPetHit& hitB);
  static double calculateTOF(double time1, double time2);
  static TVector3 calculateAnnihilationPoint(const JPetHit& hitA, const JPetHit& hitB);
  static TVector3 calculateAnnihilationPoint(const TVector3& hitA, const TVector3& hitB, double tof);
  static double calculatePlaneCenterDistance(const JPetHit& firstHit,
      const JPetHit& secondHit, const JPetHit& thirdHit);
  static bool stream2Gamma(const JPetEvent& event, JPetStatistics& stats,
                           bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff);
  static bool stream3Gamma(const JPetEvent& event, JPetStatistics& stats,
                           bool saveHistos, double d3SlotThetaMin, double d3TimeDiff, double d3DistanceFromCenter);
};

#endif /* !EVENTCATEGORIZERTOOLS_H */
