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
  bool loadOptions();
  bool isInChosenStrip(const JPetHit& hit) const;
  const char* formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix);
  void fillHistosForHit(const JPetHit& hit, const std::vector<double>& RefTimesL, const std::vector<double>& RefTimesT);
  void loadFileWithParameters(const std::string& filename);
  void saveParametersToFile(const std::string& filename);

  std::string fOutputFile = "TimeConstantsCalib.txt";
  std::string fOutputFileTmp = "TimeConstants.txt";
  const std::string fTmpOutFile = "TimeCalibLoader_ConfigFile";
  const float Cl[3] = {0., 0.1418, 0.5003};  //[ns]
  const float SigCl[3] = {0., 0.0033, 0.0033}; //[ns]
  float TOTcut[2] = { -300000000., 300000000.}; //TOT cuts for slot hits (sum of TOTs from both sides)
  const std::string fTOTcutLow  = "TOTcutLow_float";
  const std::string fTOTcutHigh  = "TOTcutHigh_float";
  const std::string kMainStripKey = "TimeWindowCreator_MainStrip_int";
  double frac_err = 0.3; //maximal fractional uncertainty of parameters accepted by calibration
  int min_ev = 100;     //minimal number of events for a distribution to be fitted
  int fLayerToCalib = 0; //Layer of calibrated slot
  int fStripToCalib = 0; //Slot to be calibrated

  float CAlTmp[5]    = {0., 0., 0., 0., 0.};
  float SigCAlTmp[5] = {0., 0., 0., 0., 0.};
  float CAtTmp[5]    = {0., 0., 0., 0., 0.};
  float SigCAtTmp[5] = {0., 0., 0., 0., 0.};
  float CBlTmp[5]    = {0., 0., 0., 0., 0.};
  float SigCBlTmp[5] = {0., 0., 0., 0., 0.};
  float CBtTmp[5]    = {0., 0., 0., 0., 0.};
  float SigCBtTmp[5] = {0., 0., 0., 0., 0.};
  float sigma_peak_Ref_lBTmp[5] = {0., 0., 0., 0., 0.};
  float sigma_peak_Ref_tBTmp[5] = {0., 0., 0., 0., 0.};
  float chi2_ndf_Ref_lBTmp[5] = {0., 0., 0., 0., 0.};
  float chi2_ndf_Ref_tBTmp[5] = {0., 0., 0., 0., 0.};
  float sigma_peak_Ref_lATmp[5] = {0., 0., 0., 0., 0.};
  float sigma_peak_Ref_tATmp[5] = {0., 0., 0., 0., 0.};
  float chi2_ndf_Ref_lATmp[5] = {0., 0., 0., 0., 0.};
  float chi2_ndf_Ref_tATmp[5] = {0., 0., 0., 0., 0.};
  int Niter = 0;
  int flag_end = 0;
  float CAtCor[5] = {0., 0., 0., 0., 0.};
  float CBtCor[5] = {0., 0., 0., 0., 0.};
  float CAlCor[5] = {0., 0., 0., 0., 0.};
  float CBlCor[5] = {0., 0., 0., 0., 0.};
  int   flag_corr = 1; //Flag for choosing the correction of times at the level of calibration module (use only if the calibration loader is not used)
  const std::string fConstantsLoadingFlag  = "ConstantsLoadingFlag";
  int   NiterMax = 1;   //Max number of iterations for calibration of one strip
  const std::string fMaxIterationNumber = "MaxIterationNumber";
  bool CheckIfExitIter(float CAl[], float  SigCAl[], float CBl[], float  SigCBl[], float CAt[], float SigCAt[], float CBt[], float SigCBt[], int Niter, int NiterM );
  std::unique_ptr<JPetGeomMapping> fMapper;
  JPetTimer fTimer;
};
#endif /*  !TimeCalibration_H */
