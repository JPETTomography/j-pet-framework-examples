/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file PALSCalibrationTask.h
 */

#ifndef PALSCALIBRATIONTASK_H
#define PALSCALIBRATIONTASK_H

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
class PALSCalibrationTask : public JPetUserTask{
public:
    PALSCalibrationTask(const char * name);
    virtual ~PALSCalibrationTask();
    virtual bool init() override;
    virtual bool exec() override;
    virtual bool terminate() override;
    void PlotTDiffAB_afterCalibration(JPetHit Hit);
    void PlotLifetimesForThresholds(JPetHit Hit1, JPetHit Hit2);
    double CorrectZPosition(JPetHit Hit);

    std::map<unsigned int, std::vector<double>> fTimeCalibrationAB;
    std::map<unsigned int, std::vector<double>> fVelocityCalibration;
    std::map<unsigned int, std::vector<double>> fTimeCalibrationPALS;

protected:
    const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
    const std::string kTOTCalculationType = "HitFinder_TOTCalculationType_std::string";
    const std::string kEventTimeWindowKey = "EventFinder_EventTime_float";
    const std::string kAnniTOTCutMinParamKey = "PALSCalibrationTask_Anni_TOT_Cut_Min_float";
    const std::string kAnniTOTCutMaxParamKey = "PALSCalibrationTask_Anni_TOT_Cut_Max_float";
    const std::string kDeexTOTCutMinParamKey = "PALSCalibrationTask_Deex_TOT_Cut_Min_float";
    const std::string kDeexTOTCutMaxParamKey = "PALSCalibrationTask_Deex_TOT_Cut_Max_float";
    const std::string kZpositionCut = "PALSCalibrationTask_ZpositionCut_float";
    const std::string kSourcePosition = "PALSCalibrationTask_SourcePosition_std::vector<double>";
    const std::string kABcorrectionFile = "PALSCalibrationTask_AB_Corrections_std::string";
    const std::string kNewVelocitiesFile = "PALSCalibrationTask_Velocities_std::string";
    const std::string kPALScorrectionFile = "PALSCalibrationTask_PALS_Corrections_std::string";
    const std::string kNmbOfThresholdsParamKey = "EventFinder_NmbOfThresholds_int";
    void saveEvents(const std::vector<JPetEvent>& event);
    bool fSaveControlHistos = true;
    std::string fTOTCalculationType = "";
    double fEventTimeWindow = 20000;
    double fAnniTOTCutMin = 20000.0;
    double fAnniTOTCutMax = 30000.0;
    double fDeexTOTCutMin = 35000.0;
    double fDeexTOTCutMax = 50000.0;
    double fZpositionCut = 25.5;
    uint fNmbOfThresholds = 4;
    std::string fABcorrectionFile = "dummyCalibration.txt";
    std::string fNewVelocitiesFile = "dummyCalibration.txt";
    std::string fPALScorrectionFile = "dummyCalibration.txt";
    TVector3 fSourcePosition;
    void initialiseHistograms();
};
#endif /* !CALIBRATIONUNIT_H */
