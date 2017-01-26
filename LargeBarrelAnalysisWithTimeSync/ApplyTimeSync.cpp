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
 *  @file TaskD.cpp
 */

#include <list>
#include <memory>
#include <fstream>
#include <JPetWriter/JPetWriter.h>
#include <JPetHitUtils/JPetHitUtils.h>
#include <JPetLargeBarrelExtensions/BarrelExtensions.h>
#include <JPetLargeBarrelExtensions/TimeSyncDeltas.h>
#include "ApplyTimeSync.h"
using namespace std;
TaskSyncAB::TaskSyncAB(const char * name, const char * description, const std::string&filename)
:LargeBarrelTask(name, description),f_filename(filename){}
TaskSyncAB::~TaskSyncAB(){}
void TaskSyncAB::init(const JPetTaskInterface::Options&opts){
    LargeBarrelTask::init(opts);
    ifstream file(f_filename);
    fSync=make_shared<Synchronization>(map(),file,defaultTimeCalculation);
    file.close();
    for(auto & layer : getParamBank().getLayers()){
	const auto ln=map()->getLayerNumber(*layer.second);
	for(size_t sl=1,n=map()->getSlotsCount(ln);sl<=n;sl++){
	    getStatistics().createHistogram( new TH1F(LayerSlot(ln,sl).c_str(), "",1200, -60.,+60.));
	}
    }
}
void TaskSyncAB::exec(){
    if(auto currHit = dynamic_cast<const JPetHit*const>(getEvent())){
	const auto strip=map()->getStripPos(currHit->getBarrelSlot());
	const auto times=fSync->get_times(*currHit);
	getStatistics().getHisto1D(LayerSlot(strip).c_str()).Fill(times.A-times.B);
    }
}
void TaskSyncAB::terminate(){}
