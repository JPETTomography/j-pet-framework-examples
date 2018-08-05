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
*  @brief Time calibration with data measured with reference detector
*  @file TimeCalibration.h
*/

#ifndef TimeCalibration_H
#define TimeCalibration_H
#include <JPetTask/JPetTask.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetTimer/JPetTimer.h>
#include <memory>
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TimeCalibration: public JPetUserTask
{
public:
  TimeCalibration(const char* name);
  virtual ~TimeCalibration();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  static constexpr int kNumberOfThresholds = 4;

  bool loadOptions();
  void loadFileWithParameters(const std::string& filename);
  void createHistograms();
  bool isInChosenStrip(const JPetHit& hit) const;
  const char* formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix);
  void fillHistosForHit(const JPetHit& hit, const std::vector<double>& RefTimesL, const std::vector<double>& RefTimesT);
  void saveParametersToFile(const std::string& filename);

  /// Required options to be loaded from the json file.
  const std::string kPMIdRefOptName  = "TimeCalibration_PMIdRef_int";
  const std::string kTOTCutLowOptName = "TimeCalibration_TOTCutLow_float";
  const std::string kTOTCutHighOptName  = "TimeCalibration_TOTCutHigh_float";
  const std::string kMainStripOptName = "TimeCalibration_MainStrip_int";
  const std::string kLoadConstantsOptName  = "TimeCalibration_LoadConstants_bool";
  const std::string kMaxIterOptName = "TimeCalibration_MaxIteration_int";
  const std::string kCalibFileTmpOptName = "TimeCalibration_OutputFileTmp_std::string";
  const std::string kCalibFileFinalOptName = "TimeCalibration_OutputFileFinal_std::string";

  int kPMIdRef = 385;
  std::array<float, 2> TOTcut{{ -300000000., 300000000.}}; //TOT cuts for slot hits (sum of TOTs from both sides)
  int fLayerToCalib = -1; //Layer of calibrated slot
  int fStripToCalib = -1; //Slot to be calibrated
  bool fIsCorrection = true; //Flag for choosing the correction of times at the level of calibration module (use only if the calibration loader is not used)
  int fMaxIter = 1;   //Max number of iterations for calibration of one strip
  std::string fTimeConstantsCalibFileName = "TimeConstantsCalib.txt";
  std::string fTimeConstantsCalibFileNameTmp = "TimeConstantsCalibTmp.txt";

  /// what is that ??
  const float Cl[3] = {0., 0.1418, 0.5003};  //[ns]
  const float SigCl[3] = {0., 0.0033, 0.0033}; //[ns]

  int Niter = 0;

  /// Structures to save the results of the calibration procedures
  /// Tmp are temporary results
  std::array<float, 5> CAlTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> SigCAlTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CAtTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> SigCAtTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CBlTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> SigCBlTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CBtTmp = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> SigCBtTmp = {{0., 0., 0., 0., 0.}};

  std::array<float, 5> CAtCor = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CBtCor = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CAlCor = {{0., 0., 0., 0., 0.}};
  std::array<float, 5> CBlCor = {{0., 0., 0., 0., 0.}};

  bool CheckIfExitIter(float CAl[], float  SigCAl[], float CBl[], float  SigCBl[], float CAt[], float SigCAt[], float CBt[], float SigCBt[], int Niter, int NiterM );
  std::unique_ptr<JPetGeomMapping> fMapper;
  JPetTimer fTimer;
};
#endif /*  !TimeCalibration_H */
