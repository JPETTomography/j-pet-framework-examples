 /**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TIMECALIBRATION_H
#define TIMECALIBRATION_H

#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * Time calibration with data measured with reference detector
 */
class TimeCalibration : public JPetUserTask {

public:
	TimeCalibration(const char * name);
  virtual ~TimeCalibration();
	virtual bool init()override;
	virtual bool exec()override;
	virtual bool terminate()override;

protected:
	void fillHistosForHit(
		const JPetHit& hit, const std::vector<double>& RefTimesL,
		const std::vector<double> & RefTimesT
	);

	const std::string fTOTcutHigh  = "TimeCalibration_TOTCutHigh_float";
	const std::string kMainStripKey = "TimeWindowCreator_MainStrip_int";
	const std::string fTOTcutLow  = "TimeCalibration_TOTCutLow_float";
	const std::string outputFile = "TimeConstantsCalib.txt";
	const std::string fCalibRunKey = "TimeCalibRunNumber";

	const float Cl[3] = {0., 0.1418, 0.5003};    //[ns]
	const float SigCl[3] = {0., 0.0033, 0.0033}; //[ns]
	//Number of calibration run associated with Acquisition Campaign
  int calibRun = 1;
	//TOT cuts for slot hits (sum of TOTs from both sides)
	float TOTcut[2] = {-300000000.,300000000.};
	//maximal fractional uncertainty of parameters accepted by calibration
	double frac_err = 0.3;
	//minimal number of events for a distribution to be fitted
	int min_ev = 100;
	//Layer of calibrated slot
	int LayerToCalib = 0;
	//Slot to be calibrated
	int StripToCalib = 0;
	float CAlTmp[4]    = {0.,0.,0.,0.};
	float SigCAlTmp[4] = {0.,0.,0.,0.};
	float CAtTemp[4]   = {0.,0.,0.,0.};
	float SigCAtTmp[4] = {0.,0.,0.,0.};
	float CBlTmp[4]    = {0.,0.,0.,0.};
	float SigCBlTmp[4] = {0.,0.,0.,0.};
	float CBtTmp[4]    = {0.,0.,0.,0.};
	float SigCBtTmp[4] = {0.,0.,0.,0.};
};
#endif /* !TIMECALIBRATION_H */
