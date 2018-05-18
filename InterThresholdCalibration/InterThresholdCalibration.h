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
 *  @file InterThresholdCalibration.h
 */
#ifndef InterThresholdCalibration_H 
#define InterThresholdCalibration_H 
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetGeomMapping/JPetGeomMapping.h>
class JPetWriter;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class InterThresholdCalibration:public JPetUserTask{
public:
	InterThresholdCalibration(const char * name);
  virtual ~InterThresholdCalibration();
	virtual bool init()override;
	virtual bool exec()override;
	virtual bool terminate()override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit & hit);
	JPetGeomMapping* fBarrelMap;
	std::string OutputFile = "TimeConstantsInterThrCalib.txt";
	const float Cl[3] = {0.,0.1418,0.5003};    //[ns]
	const float SigCl[3] = {0.,0.0033,0.0033}; //[ns]
        const std::string fCalibRunKey = "TimeCalibRunNumber_int";
        int CalibRun = 4; //Number of calibration run associated with Acquisition Campaign  
	double frac_err=0.3;  //maximal fractional uncertainty of parameters accepted by calibration
	int min_ev = 100;     //minimal number of events for a distribution to be fitted                         

	double thr_time_diff_t_A[5],thr_time_diff_A[5];
        double thr_time_diff_t_B[5],thr_time_diff_B[5];
        double lead_times_first_A,lead_times_first_B;
	double lead_times_second_A,lead_times_second_B;
	double lead_times_third_A,lead_times_third_B;
	double trail_times_first_A,trail_times_first_B;
	double trail_times_second_A,trail_times_second_B;
	double trail_times_third_A,trail_times_third_B;
        double TOT_A[5],TOT_B[5];

};
#endif /*  !InterThresholdCalibration_H */
