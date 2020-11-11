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
 *  @file SignalTransformer.h
 */

#ifndef SIGNALTRANSFORMER_H
#define SIGNALTRANSFORMER_H

#include <boost/property_tree/ptree.hpp>

#include "JPetMatrixSignal/JPetMatrixSignal.h"
#include "JPetUserTask/JPetUserTask.h"
#include <vector>

#ifdef __CINT__
#define override
#endif

class JPetWriter;

/**
 * @brief User Task: method rewriting Raw Signals to Reco and Matrix Signals.
 *
 */
class SignalTransformer: public JPetUserTask
{
public:
	SignalTransformer(const char* name);
	virtual ~SignalTransformer();
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;

protected:
	const std::string kMergeSignalsTimeParamKey = "SignalTransformer_MergeSignalsTime_double";
	const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
	const std::string kCalibBankFileParamKey = "CalibBank_FileName_std::string";
	const std::string kSaveCalibHistosParamKey = "Save_Calib_Histograms_bool";
	const std::string kConstantsFileParamKey = "ConstantsFile_std::string";
	void saveMatrixSignals(const std::vector<JPetMatrixSignal>& mtxSigVec);
	boost::property_tree::ptree fConstansTree;
	void initialiseHistograms();
	bool fSaveControlHistos = true;
	bool fSaveCalibHistos = false;
	double fMergingTime = 20000.0;
};

#endif /* !SIGNALTRANSFORMER_H */
