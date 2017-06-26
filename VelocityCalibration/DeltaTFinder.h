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
 *  @file DeltaTFinder.h
 */

#ifndef DELTATFINDER_H 
#define DELTATFINDER_H 

#include <vector>
#include <map>
#include "TF1.h"
#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
#include "../LargeBarrelAnalysisExtended/LargeBarrelMapping.h"

class JPetWriter;

#ifdef __CINT__
#	define override
#endif

class DeltaTFinder : public JPetTask{
public:
	DeltaTFinder(const char * name, const char * description);
	virtual ~DeltaTFinder(){}
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
	virtual void setWriter(JPetWriter* writer)override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit& hit);
	LargeBarrelMapping fBarrelMap;
  	bool fSaveControlHistos = true;
	JPetWriter* fWriter;
};
#endif /*  !DELTATFINDER_H */
