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
 *  @file main.cpp
 */

#include <DBHandler/HeaderFiles/DBHandler.h>
#include <JPetManager/JPetManager.h>
#include <JPetTaskLoader/JPetTaskLoader.h>
#include "TimeWindowCreator.h"
#include "TimeCalibLoader.h"
#include "SignalFinder.h"
#include "SignalTransformer.h"
#include "HitFinder.h"
#include "EventFinder.h"
#include "EventCategorizer.h"

using namespace std;

int main(int argc, char* argv[])
{
	//Connection to the remote database disabled for the moment
	//DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
	JPetManager& manager = JPetManager::getManager();
	manager.parseCmdLine(argc, argv);

	//First task - unpacking
	manager.registerTask([]() {
		return new JPetTaskLoader("hld", "tslot.raw",
				new TimeWindowCreator(
					"TimeWindowCreator",
					"Process unpacked HLD file into a tree of JPetTimeWindow objects"
				)
		);
	});

	//Second task placeholder - Signal Channel calibration
	//manager.registerTask([]() {
	//	return new JPetTaskLoader("tslot.raw", "tslot.calib",
	//		new TimeCalibLoader(
	//			"TimeCalibLoader",
	//			"Apply time corrections from prepared calibrations"
	//		)
	//	);
	//});

	//Third task - Raw Signal Creation
	
	manager.registerTask([]() {
		return new JPetTaskLoader("tslot.raw", "raw.sig",
				new SignalFinder(
					"SignalFinder",
					"Create Raw Signals, optional - draw control histograms",
                                  	true
                                )
		);
	});
	
	
	//Fourth task - Reco & Phys signal creation
	manager.registerTask([]() {
		return new JPetTaskLoader("raw.sig", "phys.sig",
				new SignalTransformer(
					"SignalTransformer",
					"Create Reco & Phys Signals"
				)
		);
	});
	

	//Fifth task - Hit construction
	manager.registerTask([]() {
		return new JPetTaskLoader("phys.sig", "hits",
				new HitFinder(
					"HitFinder",
					"Create hits from physical signals"
				)
		);
	});


	//Sixth task - unknown Event construction
	manager.registerTask([]() {
		return new JPetTaskLoader("hits", "unk.evt",
				new EventFinder(
					"EventFinder",
					"Create Events as group of Hits"
				)
		);
	});


	//Seventh task - Event Categorization
	manager.registerTask([]() {
		return new JPetTaskLoader("unk.evt", "cat.evt",
				new EventCategorizer(
					"EventCategorizer",
					"Categorize Events"
				)
		);
	});

	manager.run();
}
