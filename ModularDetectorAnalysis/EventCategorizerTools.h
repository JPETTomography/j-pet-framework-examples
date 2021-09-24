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
 *  @file EventCategorizerTools.h
 */

#ifndef EVENTCATEGORIZERTOOLS_H
#define EVENTCATEGORIZERTOOLS_H

#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetStatistics/JPetStatistics.h>
#include <TVector3.h>
#include <boost/property_tree/ptree.hpp>

static const double kLightVelocity_cm_ps = 0.0299792458;
static const double kUndefinedValue = 999.0;

/**
 * @brief Tools for Event Categorization
 *
 * Lots of tools in constatnt developement.
 */
class EventCategorizerTools
{
public:
  static bool checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff, double maxTimeDiff,
                             double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos, double scatterTestValue);

  static bool checkFor2Gamma(const JPetPhysRecoHit* firstHit, const JPetPhysRecoHit* secondHit, JPetStatistics& stats, bool saveHistos,
                             double maxThetaDiff, double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos,
                             double scatterTestValue);

  // static std::vector<JPetEvent> getLORs(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxTOF, double maxScatter,
  //                                       double totCutAnniMin, double totCutAnniMax);

  // static bool checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos);

  // static bool checkForPrompt(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double deexToTCutMin, double deexToTCutMax);

  static bool checkToT(const JPetPhysRecoHit* hit, double minToT, double maxToT);

  static bool checkRelativeAngle(const JPetBaseHit* hit1, const JPetBaseHit* hit2, double maxThetaDiff);

  // Scatter test
  static bool checkForScatter(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTestValue);

  static bool checkForScatter(const JPetBaseHit* primaryHit, const JPetBaseHit* scatterHit, JPetStatistics& stats, bool saveHistos,
                              double scatterTestValue);

  static double calculateReveresedToT(const JPetPhysRecoHit* hit);

  static double calculateDistance(const JPetBaseHit* hit1, const JPetBaseHit* hit2);

  static double calculateScatteringTime(const JPetBaseHit* hit1, const JPetBaseHit* hit2);

  static double calculateScatteringAngle(const JPetBaseHit* hit1, const JPetBaseHit* hit2);

  /// ToF is calculated as time1-time2.
  static double calculateTOF(const JPetBaseHit* hitA, const JPetBaseHit* hitB);

  static double calculateTOF(double time1, double time2);

  /// Tof calculated with the ordered hits with respect to scintillator number.
  /// The first one will be hit with smaller theta angle.
  /// See also: http://koza.if.uj.edu.pl/petwiki/index.php/Coordinate_system_in_Big_Barrel
  // cppcheck-suppress unusedFunction
  static double calculateTOFByConvention(const JPetBaseHit* hitA, const JPetBaseHit* hitB);

  static TVector3 calculateAnnihilationPoint(const JPetBaseHit* hit1, const JPetBaseHit* hit2);
  //
  // static TVector3 calculateAnnihilationPoint(const JPetBaseHit& hit1, const JPetBaseHit& hit2, const JPetBaseHit& hit3);
  //
  // static double calculatePlaneCenterDistance(const JPetBaseHit& firstHit, const JPetBaseHit& secondHit, const JPetBaseHit& thirdHit);

  // static bool stream2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff,
  //                          double maxScatter);
  //
  // static bool stream3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double d3SlotThetaMin, double d3TimeDiff,
  //                          double d3DistanceFromCenter, double maxScatter);

  // static TVector3 findIntersection(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double t21, double t31);
  //
  // static std::vector<std::vector<double>> findIntersectiosOfCircles(TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2,
  //                                                                   double R3, double R13, double R21, double R32);
  //
  // static double findMinimumFromDerivative(std::vector<double> x_vec, std::vector<double> y_vec);
};

#endif /* !EVENTCATEGORIZERTOOLS_H */
