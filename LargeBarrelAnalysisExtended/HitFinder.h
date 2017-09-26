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
 *  @file HitFinder.h
 */

#ifndef HitFinder_H
#define HitFinder_H

#include <map>
#include <vector>
#include <JPetUserTask/JPetUserTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include "HitFinderTools.h"

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for creating JPetHit from signals on oppositte photomultipliers
 *
 * This module takes all physical signals (JPetPhysSignal) within a single time window (JPetTimeWindow)
 * and creates special type of container - SignalsContainer.
 * For each scintillator with at least one signal this container stores two vectors
 * one for physical signals on photomultiplier on side A and second for signals on side B. Then
 * for each signal on side A it searches for corresponding signal on side B - that is time difference of arrival
 * of those two signals needs to be less then specified time difference (kTimeWindowWidth)
 *
 */
class HitFinder: public JPetUserTask
{

public:
	HitFinder(const char* name);
	virtual ~HitFinder();
	virtual bool init() override;
	virtual bool exec() override;
	virtual bool terminate() override;
	std::map<int, std::vector<double>> fVelocityMap;

protected:

	bool kFirstTime = true;
	HitFinderTools::SignalsContainer fAllSignalsInTimeWindow;
	HitFinderTools HitTools;
  	std::map<int, std::vector<double>> readVelocityFile();
	void fillSignalsMap(const JPetPhysSignal signal);
	void saveHits(const std::vector<JPetHit>& hits);
	const std::string fTimeWindowWidthParamKey = "HitFinder_TimeWindowWidth";
	double kTimeWindowWidth = 50000; /// in ps -> 50ns. Maximal time difference between signals

};

#endif /*  !HitFinder_H */
