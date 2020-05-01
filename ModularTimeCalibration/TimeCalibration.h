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
		const JPetHit& hit, const std::vector<double>& refTimesL,
		const std::vector<double> & refTimesT
	);

	const std::string kMainStripKey = "TimeWindowCreator_MainStrip_int";
	const std::string kTOTcutHighKey  = "TimeCalibration_TOTCutHigh_float";
	const std::string kTOTcutLowKey  = "TimeCalibration_TOTCutLow_float";
	const std::string kRefDetPMIDKey = "TimeCalib_RefDet_PM_ID_int";
	const std::string kOutputFile = "TimeConstantsCalib.txt";
	const std::string kCalibRunKey = "TimeCalib_RunNumber";
	int fNumberOfThresholds = 2;

	int fPMidRef = 385;

	const float fCl[3] = {0., 0.1418, 0.5003};    //[ns]
	const float fSigCl[3] = {0., 0.0033, 0.0033}; //[ns]
	// Number of calibration run associated with Acquisition Campaign
  int fCalibRun = 1;
	// TOT cuts for slot hits (sum of TOTs from both sides)
	float fTOTcut[2] = {-300000000., 300000000.};
	// maximal fractional uncertainty of parameters accepted by calibration
	double fFracError = 0.3;
	// Minimal number of events for a distribution to be fitted
	int fMinEvtNum = 100;
	// Strip to be calibrated - ID of a scintillator
	int fStripToCalib = 0;
	// Some temporary arrays for fitting
	float fCAlTmp[4]    = {0.,0.,0.,0.};
	float fSigCAlTmp[4] = {0.,0.,0.,0.};
	float fCAtTemp[4]   = {0.,0.,0.,0.};
	float fSigCAtTmp[4] = {0.,0.,0.,0.};
	float fCBlTmp[4]    = {0.,0.,0.,0.};
	float fSigCBlTmp[4] = {0.,0.,0.,0.};
	float fCBtTmp[4]    = {0.,0.,0.,0.};
	float fSigCBtTmp[4] = {0.,0.,0.,0.};
};

#endif /* !TIMECALIBRATION_H */
