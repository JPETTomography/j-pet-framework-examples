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
 *  @file LargeBarrelMapping.h
 */
#ifndef _LARGE_BARREL_MAPPING_
#define _LARGE_BARREL_MAPPING_
#include <map>
#include <vector>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetHit/JPetHit.h>
class LargeBarrelMapping{
public:
	
	LargeBarrelMapping();
	LargeBarrelMapping(const JPetParamBank & paramBank);
	virtual ~LargeBarrelMapping();
	int getLayerNumber(const JPetLayer & layer) const;
	int getNumberOfSlots(const JPetLayer & layer) const;
	int getNumberOfSlots(int layerNumber) const;
	int getSlotNumber(const JPetBarrelSlot & slot) const;
	int calcDeltaID(const JPetHit & hit1,const JPetHit & hit2) const;
	void buildMappings(const JPetParamBank & paramBank);
private:
	std::map<double, int> fRadiusToLayer;
	std::vector<std::map<double, int> > fThetaToSlot;
	std::vector<int> fNumberOfSlotsInLayer;
};

#endif /* _LARGE_BARREL_MAPPING_ */
