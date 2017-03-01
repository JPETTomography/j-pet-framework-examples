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
 *  @file TaskD.h
 */

#ifndef TASKD_H 
#define TASKD_H 

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include "LargeBarrelMapping.h"
class JPetWriter;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TaskD:public JPetTask{
public:
	TaskD(const char * name, const char * description);
	virtual ~TaskD(){}
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
	virtual void setWriter(JPetWriter* writer)override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit & hit);
	JPetWriter* fWriter;
	LargeBarrelMapping fBarrelMap;
        TFile *f;
        TH1F *hTOT_correction;

	//TDCHit *pHit;


  double corrA_L[3][100][5];
  double corrB_L[3][100][5];

  double corrA_T[3][100][5];
  double corrB_T[3][100][5];


};
#endif /*  !TASKD_H */
