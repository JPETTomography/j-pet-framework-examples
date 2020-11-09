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
 *
 *  @file CalibrationUnit.h
 */

#ifndef CALIBRATIONUNIT_H
#define CALIBRATIONUNIT_H

#include <JPetUserTask/JPetUserTask.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include <vector>
#include <map>

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

/**
 * @brief User Task categorizing Events
 *
 * Task attempts to add types of events to each event. Each category/type
 * has separate method for checking, if current event fulfills set of conditions.
 * These methods are defined in tools class. More than one type can be added to an event.
 * Set of controll histograms are created, unless the user decides not to produce them.
 */
class CalibrationUnit : public JPetUserTask{
public:
	CalibrationUnit(const char * name);
	virtual ~CalibrationUnit();
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
    void CalcTDiffAB_afterCalibration(JPetHit Hit);
    void PlotLifetimesForThresholds(JPetHit Hit1, JPetHit Hit2);
    
    std::vector<double> TimeShiftAB_thr1;
	std::vector<double> TimeShiftAB_thr2;
	std::vector<double> TimeShiftAB_thr3;
	std::vector<double> TimeShiftAB_thr4;

	std::vector<double> PALSTDiff_thr1;
	std::vector<double> PALSTDiff_thr2;
	std::vector<double> PALSTDiff_thr3;
	std::vector<double> PALSTDiff_thr4;

protected:
	const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
    const std::string kTOTCalculationType = "HitFinder_TOTCalculationType_std::string";
	const std::string kAnniTOTCutMinParamKey = "CalibrationUnit_Anni_TOT_Cut_Min_float";
	const std::string kAnniTOTCutMaxParamKey = "CalibrationUnit_Anni_TOT_Cut_Max_float";
	const std::string kDeexTOTCutMinParamKey = "CalibrationUnit_Deex_TOT_Cut_Min_float";
	const std::string kDeexTOTCutMaxParamKey = "CalibrationUnit_Dee_TOT_Cut_Max_float";
    const std::string kZpositionCut = "CalibrationUnit_ZpositionCut_float";
    const std::string kEffectiveLength = "CalibrationUnit_EffectiveLength_float";
    const std::string kSourcePosition = "CalibrationUnit_SourcePosition_std::vector<double>";
    const std::string kABcorrectionFile = "CalibrationUnit_AB_Corrections";
    const std::string kPALScorrectionFile = "CalibrationUnit_PALS_Corrections";
	void saveEvents(const std::vector<JPetEvent>& event);
	bool fSaveControlHistos = true;
    std::string fTOTCalculationType = "";
	double fAnniTOTCutMin = 20000.0;
	double fAnniTOTCutMax = 30000.0;
	double fDeexTOTCutMin = 35000.0;
	double fDeexTOTCutMax = 50000.0;
    double fZpositionCut = 25.5;
    double fEffectiveLength = 50.;
    std::string fABcorrectionFile = "";
    std::string fPALScorrectionFile = "";
    double fSpeedOfLight = 29.979246;
    TVector3 fSourcePosition;
	void initialiseHistograms();
};
#endif /* !CALIBRATIONUNIT_H */
