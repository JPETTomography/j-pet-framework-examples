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
 *  @file CalibrationTools.h
 */

#ifndef CALIBRATIONTOOLS_H
#define CALIBRATIONTOOLS_H

#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetStatistics/JPetStatistics.h>
#include <TVector3.h>
#include <boost/property_tree/ptree.hpp>

/**
 * Average velocity of cosmic muons at the sea level
 * http://web.mit.edu/lululiu/Public/pixx/not-pixx/muons.pdf
 */
static const double kMuonAvVelocity_cm_ps = 0.0299792458;

/**
 * @brief Tools for managing calibrations
 */
class CalibrationTools
{

public:
  static void selectForTOF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                           double totCutDeexMin, double totCutDeexMax, boost::property_tree::ptree& calibTree);

  static void selectForTOF2Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                                 double totCutDeexMin, double totCutDeexMax, double scatterTestValue);

  static void selectForTOF3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                                 double totCutDeexMin, double totCutDeexMax, double scatterTestValue);

  static void selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double maxTimeDiff,
                                double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos);

  static void selectCosmicsForToF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double detectorYRot);

  static double calculateReveresedToT(const JPetPhysRecoHit* hit);
};

#endif /* !CALIBRATIONTOOLS_H */
