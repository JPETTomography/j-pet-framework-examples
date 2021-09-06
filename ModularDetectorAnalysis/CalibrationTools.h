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

#include <JPetEvent/JPetEvent.h>
#include <JPetStatistics/JPetStatistics.h>
#include <TVector3.h>

/**
 * @brief Tools for managing calibrations
 *
 */
class CalibrationTools
{

public:
  static void selectForTOF(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double totCutAnniMin, double totCutAnniMax,
                           double totCutDeexMin, double totCutDeexMax, const TVector3& sourcePos, double scatterTestValue);

  static void selectForTimeWalk(const JPetEvent& event, JPetStatistics& stats, bool saveCalibHistos, double maxThetaDiff, double maxTimeDiff,
                                double totCutAnniMin, double totCutAnniMax, const TVector3& sourcePos);
};

#endif /* !CALIBRATIONTOOLS_H */
