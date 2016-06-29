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

#include "JPetParamBank/JPetParamBank.h"
#include <map>
#include <algorithm>

class LargeBarrelMapping{

 public:
  
  LargeBarrelMapping(){};
  
  LargeBarrelMapping(const JPetParamBank & paramBank){
    
    buildMappings(paramBank);
    
  }

  int getLayerNumber(const JPetLayer & layer) const{
    return fRadiusToLayer.at( layer.getRadius() );
  }
  
  int getNumberOfSlots(const JPetLayer & layer) const {
    return fNumberOfSlotsInLayer.at(fRadiusToLayer.at( layer.getRadius() ) - 1);
  }

  int getNumberOfSlots(int layerNumber) const {
    return fNumberOfSlotsInLayer.at(layerNumber-1);
  }

  
  int getSlotNumber(const JPetBarrelSlot & slot) const {
    return fThetaToSlot[getLayerNumber(slot.getLayer())-1].at(slot.getTheta());
  }

  int calcDeltaID(JPetHit & hit1, JPetHit & hit2) const{
    
    int delta_ID = abs( getSlotNumber(hit1.getBarrelSlot()) -
			getSlotNumber(hit2.getBarrelSlot()) );
    int half_layer_size = getNumberOfSlots(hit1.getBarrelSlot().getLayer());
    if( delta_ID > half_layer_size ){
      delta_ID = half_layer_size - delta_ID;
    }
    return delta_ID;
  }

  
  void buildMappings(const JPetParamBank & paramBank){

    std::vector<double> layersRadii;
    std::vector<std::vector<double> > slotsTheta;
    
    // fill layers' radii vector for sorting
    for(auto & layer : paramBank.getLayers() ){
      double radius = layer.second->getRadius();
      
      layersRadii.push_back(radius);

      fNumberOfSlotsInLayer.push_back(0);

      std::map<double, int> slots_map;
      fThetaToSlot.push_back(slots_map);
      std::vector<double> slots_theta;
      slotsTheta.push_back(slots_theta);
      
    }

    // sort by radius
    std::sort( layersRadii.begin(), layersRadii.end(), std::less<double>() );
    // number layers by increasing radius starting from 1
    int layer_counter = 1;
    for(auto & radius : layersRadii ){
      fRadiusToLayer[ radius ] = layer_counter++;
    }

    // count barrel slots in each layer
    // and fill theta angle to slot number mapping
    for(auto & slot : paramBank.getBarrelSlots()){

      int layer_number = getLayerNumber( slot.second->getLayer() );
      fNumberOfSlotsInLayer[layer_number-1]++;
      slotsTheta[layer_number-1].push_back(slot.second->getTheta());
      
    }

    // for each layer, sort slot theta angles
    // and number the slots by increasing theta starting from 1
    std::vector<int> slotCounters;
    int layerNumber = 1;
    for(auto & thetas : slotsTheta){
      slotCounters.push_back(1);
      std::sort( thetas.begin(), thetas.end(), std::less<double>() );
      for(double theta : thetas){
	fThetaToSlot[layerNumber-1][theta] = slotCounters[layerNumber-1]++;
      }
      layerNumber++;
    }
    
    
  }

 private:
  
  std::map<double, int> fRadiusToLayer;
  std::vector<std::map<double, int> > fThetaToSlot;
  std::vector<int> fNumberOfSlotsInLayer;

  const double degFullCircle = 360.;
  
};

#endif /* _LARGE_BARREL_MAPPING_ */
