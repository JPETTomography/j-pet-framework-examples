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
 *  @file Module2.h
 */

#ifndef MODULE2_H
#define MODULE2_H

#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include "LargeBarrelMapping.h"

using namespace std;
class JPetWriter;

#ifdef __CINT__
#define override
#endif

class Module2: public JPetTask {
	public:
		Module2(const char * name, const char * description);
		virtual ~Module2();
		virtual void init(const JPetTaskInterface::Options& opts) override;
		virtual void exec() override;
		virtual void terminate() override;
		virtual void setWriter(JPetWriter* writer) override;
		virtual void setParamManager(JPetParamManager* paramManager) override;
		const JPetParamBank& getParamBank()const;

	protected:
		JPetWriter* fWriter;
		JPetParamManager* fParamManager;
		LargeBarrelMapping fBarrelMap;
		void saveRawSignals(vector<JPetRawSignal> sigChVec);
		vector<JPetRawSignal> buildRawSignals(Int_t timeWindowIndex, vector<JPetSigCh> sigChFromSamePM);
		int findSigChOnNextThr(JPetSigCh sigCh, vector<JPetSigCh> sigChVec);
		int findTrailingSigCh(JPetSigCh leadingSigCh, vector<JPetSigCh> trailingSigChVec);
		const int kNumOfThresholds = 4;
		const Float_t SIGCH_EDGE_MAX_TIME = 20000; //[ps]
		const Float_t SIGCH_LEAD_TRAIL_MAX_TIME = 300000; //[ps]
};
#endif 
/*  !MODULE2_H */
