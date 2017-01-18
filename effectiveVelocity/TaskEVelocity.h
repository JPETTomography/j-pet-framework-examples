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
 *  @file TaskE.h
 */
#ifndef TASKE_VELOCITY_H
#define TASKE_VELOCITY_H
#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include "LargeBarrelMapping.h"
#include <vector>
class JPetWriter;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TaskEVelocity:public JPetTask {
public:
	TaskEVelocity(const char * name, const char * description);
	virtual ~TaskEVelocity();
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
	virtual void setWriter(JPetWriter* writer)override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillCoincidenceHistos(const std::vector<JPetHit>& hits);
	void fillDeltaIDhisto(int delta_ID, int threshold, const JPetLayer & layer);
	void fillTOFvsDeltaIDhisto(int delta_ID, int threshold, const JPetHit & hit1, const JPetHit & hit2);
	bool isGoodTimeDiff(const JPetHit & hit, int thr);
	void fillTOTvsTOThisto(int delta_ID, int thr, const JPetHit & hit1, const JPetHit & hit2);
	void fillTOTnoCoin(int delta_ID, int thr,const JPetHit& hit,char side);
	void fillTOTwithCoin(int delta_ID, int thr,const JPetHit& hit,char side);
	void fillDeltaT(int delta_ID, int thr,const JPetHit& hit);
	void setSlotsAndCuts();
	void fillDeltaTvsTOT( int thr, const JPetHit& hit);
private:
	LargeBarrelMapping fBarrelMap;
	std::vector<JPetHit> fHits;
	JPetWriter* fWriter;
	std::vector<int> slots;
	std::vector<double> TOTcuts;
};
#endif /*  !TASKE_H */
