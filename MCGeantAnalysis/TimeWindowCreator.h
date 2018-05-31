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
 *  @file TimeWindowCreator.h
 */

#ifndef TimeWindowCreator_H
#define TimeWindowCreator_H

#include <JPetUserTask/JPetUserTask.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetTOMBChannel/JPetTOMBChannel.h>
#include <set>
#include <map>

class JPetWriter;

#ifdef __CINT__
#define override
#endif

/**
 * @brief User Task: translate Unpacker EventIII data to JPetTimeWindow
 *
 * Task translates data from Unpacker file fomrat - EventIII to JPetTimeWindow.
 * Parameters for start and end time can be specified in user options, default are
 * provided. Moreover time calibration and threshold values injection can be
 * performed, if ASCII files of standard format were provided. In case of errors,
 * creation of Time Windows continues without this additional information.
 */

class TimeWindowCreator: public JPetUserTask
{
public:
	TimeWindowCreator(const char* name);
	virtual ~TimeWindowCreator();
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;

protected:
	bool filter(const JPetTOMBChannel& channel) const;
	JPetSigCh generateSigCh(const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge) const;
	const std::string kMainStripKey = "TimeWindowCreator_MainStrip_int";
	const std::string kMaxTimeParamKey = "TimeWindowCreator_MaxTime_float";
	const std::string kMinTimeParamKey = "TimeWindowCreator_MinTime_float";
	const std::string kTimeCalibFileParamKey = "TimeCalibLoader_ConfigFile_std::string";
	const std::string kThresholdFileParamKey = "ThresholdLoader_ConfigFile_std::string";
	const std::string kSaveControlHistosParamKey = "Save_Cotrol_Histograms_bool";
	std::map<unsigned int, std::vector<double>> fTimeCalibration;
	std::map<unsigned int, std::vector<double>> fThresholds;
	std::pair<int,int> fMainStrip;
	std::set<int> fAllowedChannels;
	long long int fCurrEventNumber = 0;
	bool fSaveControlHistos = true;
	bool fMainStripSet = false;
	double fMinTime = -1.e6;
	double fMaxTime = 0.;
};

#endif /*  !TimeWindowCreator_H */
