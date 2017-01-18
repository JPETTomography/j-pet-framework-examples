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
 *  @file Calib1.h
 */

#ifndef Calib1_H 
#define Calib1_H 

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
class Calib1:public JPetTask{
public:
	Calib1(const char * name, const char * description);
	virtual ~Calib1(){}
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
	virtual void setWriter(JPetWriter* writer)override;
protected:
	const char * formatUniqueSlotDescription(const JPetBarrelSlot & slot, int threshold,const char * prefix);
	void fillHistosForHit(const JPetHit & hit,const std::vector<double> &RefTimesL,const std::vector<double> & RefTimesT);
	std::vector<JPetHit> hitsCalib;
	std::vector <double> RefTimesL;
	std::vector <double> RefTimesT;
	JPetWriter* fWriter;
	LargeBarrelMapping fBarrelMap;
};
#endif /*  !Calib1_H */
