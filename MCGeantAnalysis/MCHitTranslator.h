/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file MCHitTranslator.h
 */

#ifndef MCHitTranslator_H
#define MCHitTranslator_H

#include <map>
#include <vector>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for creating JPetHit from signals on oppositte photomultipliers
 *
 *
 */
class MCHitTranslator: public JPetUserTask
{

public:
  MCHitTranslator(const char* name);
  virtual ~MCHitTranslator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;


protected :

//  bool kFirstTime = true;
//  MCHitTranslatorTools::SignalsContainer fAllSignalsInTimeWindow;
//  MCHitTranslatorTools fHitTools;
//
//  void fillSignalsMap(const JPetPhysSignal& signal);
//  void saveHits(const std::vector<JPetHit>& hits);
//  const std::string fTimeWindowWidthParamKey = "MCHitTranslator_TimeWindowWidth_float";
//  const std::string fVelocityCalibFileParamKey = "MCHitTranslator_Velocity_Calibration_File_Path_std::string";
//  double kTimeWindowWidth = 50000; /// in ps -> 50ns. Maximal time difference between signals
//
//private:
//  std::string fVelocityCalibrationFilePath = "resultsForThresholda.txt";
//  MCHitTranslatorTools::VelocityMap fVelocityMap;
//
//  MCHitTranslatorTools::VelocityMap readVelocityFile();
};

#endif 
