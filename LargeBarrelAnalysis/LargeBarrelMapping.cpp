#include <algorithm>
#include "LargeBarrelMapping.h"
using namespace std;
const double degFullCircle = 360.;

LargeBarrelMapping::LargeBarrelMapping(){}
LargeBarrelMapping::~LargeBarrelMapping(){}
LargeBarrelMapping::LargeBarrelMapping(const JPetParamBank& paramBank){
	buildMappings(paramBank);
}
int LargeBarrelMapping::getLayerNumber(const JPetLayer& layer) const{
	return fRadiusToLayer.at(layer.getRadius());//??????? it's very strange thing
}
int LargeBarrelMapping::getNumberOfSlots(const JPetLayer& layer) const {
	return fNumberOfSlotsInLayer.at(fRadiusToLayer.at(layer.getRadius()) - 1);//??????? it's very strange thing
}
int LargeBarrelMapping::getNumberOfSlots(int layerNumber) const {
	return fNumberOfSlotsInLayer.at(layerNumber-1);
}
int LargeBarrelMapping::getSlotNumber(const JPetBarrelSlot& slot) const{
	return fThetaToSlot[getLayerNumber(slot.getLayer())-1].at(slot.getTheta());
}
int LargeBarrelMapping::calcDeltaID(const JPetHit& hit1,const JPetHit& hit2) const{
	if(hit1.getBarrelSlot().getLayer().getID()==hit2.getBarrelSlot().getLayer().getID()){
		int delta_ID = abs(getSlotNumber(hit1.getBarrelSlot())-getSlotNumber(hit2.getBarrelSlot()));
		int layer_size = getNumberOfSlots(hit1.getBarrelSlot().getLayer());
		int half_layer_size = layer_size/2;
		if( delta_ID > half_layer_size ) return layer_size-delta_ID;
		return delta_ID;
	}
	return -1;//maybe throwing an exception would be a better solution?
}
void LargeBarrelMapping::buildMappings(const JPetParamBank& paramBank){
	vector<double> layersRadii;
	vector<vector<double> > slotsTheta;
	for(auto & layer : paramBank.getLayers() ){
		double radius = layer.second->getRadius();
		layersRadii.push_back(radius);
		fNumberOfSlotsInLayer.push_back(0);
		map<double, int> slots_map;
		fThetaToSlot.push_back(slots_map);
		vector<double> slots_theta;
		slotsTheta.push_back(slots_theta);
	}
	sort( layersRadii.begin(), layersRadii.end(),less<double>() );
	int layer_counter = 1;
	for(const auto & radius : layersRadii ){
		fRadiusToLayer[ radius ] = layer_counter++;
	}
	for(const auto & slot : paramBank.getBarrelSlots()){
		int layer_number = getLayerNumber( slot.second->getLayer() );
		fNumberOfSlotsInLayer[layer_number-1]++;
		slotsTheta[layer_number-1].push_back(slot.second->getTheta());
	}
	vector<int> slotCounters;
	int layerNumber = 1;
	for(auto & thetas : slotsTheta){
		slotCounters.push_back(1);
		sort( thetas.begin(), thetas.end(), less<double>() );
		for(const double&theta : thetas){
			fThetaToSlot[layerNumber-1][theta] = slotCounters[layerNumber-1]++;
		}
		layerNumber++;
	}
}
