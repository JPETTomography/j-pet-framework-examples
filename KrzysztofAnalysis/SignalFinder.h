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
 *  @file SignalFinder.h
 */

#ifndef SIGNALFINDER_H
#define SIGNALFINDER_H

#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include "LargeBarrelMapping.h"

class JPetWriter;

#ifdef __CINT__
#define override
#endif

class SignalFinder: public JPetTask {
	public:
		SignalFinder(const char * name, const char * description, bool printStats);
		virtual ~SignalFinder();
		virtual void init(const JPetTaskInterface::Options& opts) override;
		virtual void exec() override;
		virtual void terminate() override;
		virtual void setWriter(JPetWriter* writer) override;
		virtual void setParamManager(JPetParamManager* paramManager) override;
		const JPetParamBank& getParamBank()const;
		bool fSaveControlHistos = true;

	protected:
		JPetWriter* fWriter;
		JPetParamManager* fParamManager;
		LargeBarrelMapping fBarrelMap;
		void saveRawSignals(const std::vector<JPetRawSignal> & sigChVec);
		std::map<int,std::vector<JPetSigCh>> getSigChsPMMapById(const JPetTimeWindow* timeWindow);
		std::vector<JPetRawSignal> buildAllSignals(Int_t timeWindowIndex,
					std::map<int,std::vector<JPetSigCh>> sigChsPMMap);
		std::vector<JPetRawSignal> buildRawSignals(Int_t timeWindowIndex,
					const std::vector<JPetSigCh> & sigChFromSamePM);
		int findSigChOnNextThr(Double_t sigChValue, const std::vector<JPetSigCh> & sigChVec);
		int findTrailingSigCh(JPetSigCh leadingSigCh,
				      const std::vector<JPetSigCh> & trailingSigChVec);
		const int kNumOfThresholds = 4;
		const Float_t kSigChEdgeMaxTime = 20000; //[ps]
		const Float_t kSigChLeadTrailMaxTime = 300000; //[ps]
};
#endif
/*  !SIGNALFINDER_H */
