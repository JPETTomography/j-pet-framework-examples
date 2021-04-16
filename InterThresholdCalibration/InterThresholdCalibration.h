/**
*  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
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
* Purpose: Time calibration with data measured with reference detector
*  @file InterThresholdCalibration.h
*/
#ifndef InterThresholdCalibration_H
#define InterThresholdCalibration_H
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
#include <JPetTimer/JPetTimer.h>
class JPetWriter;
class InterThresholdCalibration: public JPetUserTask
{
public:
  InterThresholdCalibration(const char* name);
  virtual ~InterThresholdCalibration();
  virtual bool init()override;
  virtual bool exec()override;
  virtual bool terminate()override;
protected:
  const char* formatUniqueSlotDescription(const JPetBarrelSlot& slot, int threshold, const char* prefix);
  void fillHistosForHit(const JPetHit& hit);
  JPetGeomMapping* fBarrelMap = nullptr;
  std::string fOutputFile = "TimeConstantsInterThrCalib.txt";
  const std::string fOutputFileKey = "InterThresholdCalibration_TimeConstantsInterThrCalibOutputFile_std::string";
  const std::string fCalibRunKey = "InterThresholdCalibration_TimeCalibRunNumber_int";
  int fCalibRun = 4; //Number of calibration run associated with Acquisition Campaign
  double fFrac_err = 0.3; //maximal fractional uncertainty of parameters accepted by calibration
  const std::string fFrac_errKey = "InterThresholdCalibration_Frac_err_double";
  int fMin_ev = 100;     //minimal number of events for a distribution to be fitted
  const std::string fMin_evKey = "InterThresholdCalibration_Min_ev_int";
  std::vector<double> kSl_max; //amount of slots per each layer
  double fThr_time_diff_t_A[5], fThr_time_diff_A[5];
  double fThr_time_diff_t_B[5], fThr_time_diff_B[5];

};
#endif /*  !InterThresholdCalibration_H */
