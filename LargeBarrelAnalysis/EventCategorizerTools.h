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

#include <JPetStatistics/JPetStatistics.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>

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
  static bool checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats,
                           bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff);
  static bool checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos);
  static bool checkForPrompt(const JPetEvent& event, JPetStatistics& stats,
                             bool saveHistos, double deexTOTCutMin, double deexTOTCutMax, 
                             std::string fTOTCalculationType);
  static bool checkForScatter(const JPetEvent& event, JPetStatistics& stats,
                              bool saveHistos, double scatterTOFTimeDiff, 
                              std::string fTOTCalculationType);
  static double calculateDistance(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2);
  /// Tof is calculated as  time1 -time2.
  static double calculateTOF(const JPetHit& hitA, const JPetHit& hitB);
  static double calculateTOF(double time1, double time2);
  /// Tof calculated with the ordered hits with respect to scintillator number.
  /// The first one will be hit with smaller theta angle.
  /// See also: http://koza.if.uj.edu.pl/petwiki/index.php/Coordinate_system_in_Big_Barrel

  // cppcheck-suppress unusedFunction
  static double calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB);
  static TVector3 calculateAnnihilationPoint(const JPetHit& hitA, const JPetHit& hitB);
  static TVector3 calculateAnnihilationPoint(const TVector3& hitA, const TVector3& hitB, double tof);
  static double calculatePlaneCenterDistance(const JPetHit& firstHit,
      const JPetHit& secondHit, const JPetHit& thirdHit);

};

#endif /* !EVENTCATEGORIZERTOOLS_H */
