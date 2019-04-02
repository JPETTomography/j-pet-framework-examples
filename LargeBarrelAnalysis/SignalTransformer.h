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
 *  @file SignalTransformer.h
 */

#ifndef SIGNALTRANSFORMER_H
#define SIGNALTRANSFORMER_H

#include "JPetRecoSignal/JPetRecoSignal.h"
#include "JPetUserTask/JPetUserTask.h"

#ifdef __CINT__
#define override
#endif

class JPetWriter;

/**
 * @brief User Task: method rewriting Raw Signals to Reco and Phys Signals.
 *
 * Task rewrites Raw Signals to Reco Signals and Physical Signals, saving JPetPhysSignal.
 * Only time of the signal is set, the rest of available fields are set to -1,
 * also using corrupted signal, if indicated by user.
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
	void initialiseHistograms();
	JPetRecoSignal createRecoSignal(const JPetRawSignal& rawSignal);
	JPetPhysSignal createPhysSignal(const JPetRecoSignal& signals);
	void correctForWalk(const JPetRecoSignal& recoSignal);
	const std::string kUseCorruptedSignalsParamKey = "SignalTransformer_UseCorruptedSignals_bool";
	const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
	const std::string kWalkCorrConst1ParamKey = "SignalTransformer_WalkCorrConstThr1_float";
	const std::string kWalkCorrConst2ParamKey = "SignalTransformer_WalkCorrConstThr2_float";
        const std::string kWalkCorrConst3ParamKey = "SignalTransformer_WalkCorrConstThr3_float";
	const std::string kWalkCorrConst4ParamKey = "SignalTransformer_WalkCorrConstThr4_float";
	bool fUseCorruptedSignals = false;
	bool fSaveControlHistos = true;
	double fWalkCorrConst[4] = {0.,0.,0,0.};
};
#endif /* !SIGNALTRANSFORMER_H */
