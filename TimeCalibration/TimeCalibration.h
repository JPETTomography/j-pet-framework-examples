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
 * Purpose: Time calibration with data measured with reference detector
 *  @file TimeCalibration.h
 */

#ifndef TimeCalibration_H 
#define TimeCalibration_H 
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include "LargeBarrelMapping.h"
class JPetWriter;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TimeCalibration:public JPetUserTask{
public:
	TimeCalibration(const char * name);
	virtual ~TimeCalibration(){}
	virtual bool init()override;
	virtual bool exec()override;
	virtual bool terminate()override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit & hit,const std::vector<double> &RefTimesL,const std::vector<double> & RefTimesT);
	LargeBarrelMapping fBarrelMap;
	std::string OutputFile = "TimeConstantsCalib.txt";
	const float Cl[3] = {0.,0.1418,0.5003};    //[ns]
	const float SigCl[3] = {0.,0.0033,0.0033}; //[ns]
        const std::string fCalibRunKey = "TimeCalibRunNumber";
        int CalibRun = 1; //Number of calibration run associated with Acquisition Campaign  
	float TOTcut[2] = {-300000000.,300000000.}; //TOT cuts for slot hits (sum of TOTs from both sides)
	const std::string fTOTcutLow  = "TOTcutLow";
	const std::string fTOTcutHigh  = "TOTcutHigh";
	const std::string kMainStripKey = "TimeWindowCreator_MainStrip";
	double frac_err=0.3;  //maximal fractional uncertainty of parameters accepted by calibration
	int min_ev = 100;     //minimal number of events for a distribution to be fitted                         
	int LayerToCalib = 0; //Layer of calibrated slot
	int StripToCalib = 0; //Slot to be calibrated
	float CAlTmp[4]    = {0.,0.,0.,0.};
	float SigCAlTmp[4] = {0.,0.,0.,0.};
	float CAtTemp[4]   = {0.,0.,0.,0.};
	float SigCAtTmp[4] = {0.,0.,0.,0.};
	float CBlTmp[4]    = {0.,0.,0.,0.};
	float SigCBlTmp[4] = {0.,0.,0.,0.};
	float CBtTmp[4]    = {0.,0.,0.,0.};
	float SigCBtTmp[4] = {0.,0.,0.,0.};
};
#endif /*  !TimeCalibration_H */
