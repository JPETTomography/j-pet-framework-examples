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
 *  @file EventCategorizerTools.h
 */

#ifndef EVENTCATEGORIZERTOOLS_H
#define EVENTCATEGORIZERTOOLS_H

#include <TVector3.h>
#include <boost/property_tree/ptree.hpp>

#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <JPetStatistics/JPetStatistics.h>

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
  static void selectForTOF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                           double totCutDeexMin, double totCutDeexMax, const TVector3& sourcePos);

  static void selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double maxTimeDiff,
                                double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos);

  static std::vector<JPetEvent> getLORs(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxTOF, double maxScatter,
                                        double totCutAnniMin, double totCutAnniMax);

  static bool checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double maxThetaDiff, double maxTimeDiff,
                             double totCutAnniMin, double totCutAnniMax, double lorAngleMax, double lorPosZMax, const TVector3& sourcePos);

  static bool checkFor2Gamma(const JPetHit& firstHit, const JPetHit& secondHit, JPetStatistics& stats, bool saveHistos, double maxThetaDiff,
                             double maxTimeDiff, double totCutAnniMin, double totCutAnniMax, double lorAngleMax, double lorPosZMax,
                             const TVector3& sourcePos);

  static bool checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos);

  static bool checkForPrompt(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double deexTOTCutMin, double deexTOTCutMax);

  // Scatter test
  static bool checkForScatter(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff);

  static bool checkForScatter(const JPetHit& primaryHit, const JPetHit& scatterHit, JPetStatistics& stats, bool saveHistos,
                              double scatterTOFTimeDiff);

  // Helper methods - some obsolete in modular approach, TODO check
  static double calculateTOT(const JPetHit& hit);

  static double calculateReveresedTOT(const JPetHit& hit);

  static double calculateDistance(const JPetHit& hit1, const JPetHit& hit2);

  static double calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2);

  static double calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2);

  /// ToF is calculated as time1-time2.
  static double calculateTOF(const JPetHit& hitA, const JPetHit& hitB);

  static double calculateTOF(double time1, double time2);

  /// Tof calculated with the ordered hits with respect to scintillator number.
  /// The first one will be hit with smaller theta angle.
  /// See also: http://koza.if.uj.edu.pl/petwiki/index.php/Coordinate_system_in_Big_Barrel
  // cppcheck-suppress unusedFunction
  static double calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB);

  static TVector3 calculateAnnihilationPoint(const JPetHit& hit1, const JPetHit& hit2);

  static TVector3 calculateAnnihilationPoint(const JPetHit& hit1, const JPetHit& hit2, const JPetHit& hit3);

  static double calculatePlaneCenterDistance(const JPetHit& firstHit, const JPetHit& secondHit, const JPetHit& thirdHit);

  static bool stream2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff);

  static bool stream3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double d3SlotThetaMin, double d3TimeDiff,
                           double d3DistanceFromCenter);

  static TVector3 findIntersection(TVector3 hit1Pos, TVector3 hit2Pos, TVector3 hit3Pos, double t21, double t31);

  static std::vector<std::vector<double>> findIntersectiosOfCircles(TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2,
                                                                    double R3, double R13, double R21, double R32);

  static double findMinimumFromDerivative(std::vector<double> x_vec, std::vector<double> y_vec);
};

#endif /* !EVENTCATEGORIZERTOOLS_H */
