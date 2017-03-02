/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file SignalFinderTools.h
 */

#ifndef SIGNALFINDERTOOLS_H
#define SIGNALFINDERTOOLS_H
#include <vector>
#include <map>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetSigCh/JPetSigCh.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetStatistics/JPetStatistics.h>

class SignalFinderTools
{
public:

	//Method returns a map of vectors of JPetSigCh ordered by photomultiplier id.
	//The map is based on the JPetSigCh from a given timeWindow.
	static std::map<int, std::vector<JPetSigCh>> getSigChsPMMapById(const JPetTimeWindow* timeWindow);

	//Method reconstructs all signals based on the signal channels
	//from the SigChPMMap
	static std::vector<JPetRawSignal> buildAllSignals(
  				Int_t timeWindowIndex,
				std::map<int, std::vector<JPetSigCh>> sigChsPMMap,
				int numOfThresholds,
				JPetStatistics& stats,
				bool saveControlHistos,
				double sigChEdgeMaxTime,
				double sigChLeadTrailMaxTime
	);

	//Method reconstructs signals based on the signal channels
	//from the sigChFromSamePM container
	static std::vector<JPetRawSignal> buildRawSignals(Int_t timeWindowIndex,
				const std::vector<JPetSigCh>& sigChFromSamePM,
				int numOfThresholds,
				JPetStatistics& stats,
				bool saveControlHistos,
				double sigChEdgeMaxTime,
				double sigChLeadTrailMaxTime
	);

  	//Methods for checking relative between Signal Channel times
	//and if they fit in defined time windows
	static int findSigChOnNextThr(Double_t sigChValue,
				const std::vector<JPetSigCh>& sigChVec,
				double sigChEdgeMaxTime);

	static int findTrailingSigCh(const JPetSigCh& leadingSigCh,
				const std::vector<JPetSigCh>& trailingSigChVec,
				double sigChLeadTrailMaxTime);

};
#endif /*  !SIGNALFINDERTOOLS_H */
