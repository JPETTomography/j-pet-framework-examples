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
 *  @file TaskCVelocity.h
 */

#ifndef TASKC_VELOCITY_H
#define TASKC_VELOCITY_H 

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
class JPetWriter;
#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif
class TaskCVelocity:public JPetTask {
public:
	TaskCVelocity(const char * name, const char * description);
	virtual ~TaskCVelocity();
	virtual void init(const JPetTaskInterface::Options& opts)override;
	virtual void exec()override;
	virtual void terminate()override;
	virtual void setWriter(JPetWriter* writer)override;
protected:
	std::vector<JPetHit> createHits(const std::vector<JPetRawSignal>& signals);
	void saveHits(const std::vector<JPetHit>&hits);
	std::vector<JPetRawSignal> fSignals;
	JPetWriter* fWriter;
};
#endif /*  !TASKD_H */
