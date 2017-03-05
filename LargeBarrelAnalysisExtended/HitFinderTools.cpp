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
 *  @file HitFinderTools.h
 */

#include "HitFinderTools.h"

using namespace std;

vector<JPetHit> HitFinderTools::createHits(const SignalsContainer& allSignalsInTimeWindow,
						const double timeDifferenceWindow)
{
	vector<JPetHit> hits;

	for (auto scintillator : allSignalsInTimeWindow) {

		auto sideA = scintillator.second.first;
		auto sideB = scintillator.second.second;

		if (sideA.size() > 0 && sideB.size() > 0) {

			std::sort(sideA.begin(),
				sideA.end(),
				[] (const JPetPhysSignal & h1,
					const JPetPhysSignal & h2) {
					return h1.getTime() < h2.getTime();
				});

			std::sort(sideB.begin(),
				sideB.end(),
				[] (const JPetPhysSignal & h1,
					const JPetPhysSignal & h2) {
				        return h1.getTime() < h2.getTime();
				});

			for (auto signalA : sideA) {
				for (auto signalB : sideB) {

					if ((signalB.getTime() - signalA.getTime())
								> timeDifferenceWindow)
									break;

					if (fabs(signalA.getTime() - signalB.getTime())
								< timeDifferenceWindow) {

						//Creating hit for successfully matched pair of Phys singlas
						//Setting meaningless parameters of Energy, Position, quality
						JPetHit hit;
						hit.setSignalA(signalA);
						hit.setSignalB(signalB);
						hit.setTime((signalA.getTime()
								+ signalB.getTime())/2.0);
						hit.setQualityOfTime(-1.0);
						hit.setTimeDiff(fabs(signalA.getTime()
								- signalB.getTime()));
						hit.setQualityOfTimeDiff(-1.0);
						hit.setEnergy(-1.0);
						hit.setQualityOfEnergy(-1.0);

            			hit.setScintillator(signalA.getPM().getScin());
						hit.setBarrelSlot(signalA.getPM().getBarrelSlot());

						hits.push_back(hit);
					}
				}
			}
		}
	}
	return hits;
}
