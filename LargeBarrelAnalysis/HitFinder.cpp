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
 *  @file HitFinder.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include <JPetAnalysisTools/JPetAnalysisTools.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include "HitFinder.h"
#include "HitFinderTools.h"

using namespace jpet_options_tools;

using namespace std;

HitFinder::HitFinder(const char* name): JPetUserTask(name) {}

HitFinder::~HitFinder() {}

bool HitFinder::init()
{
  fOutputEvents = new JPetTimeWindow("JPetHit");
  
  INFO("Reading velocities.");
	fVelocityMap = readVelocityFile();

  getStatistics().createHistogram(
    new TH1F("hits_per_time_window",
      "Number of Hits in Time Window",
      101, -0.5, 100.5
    )
  );

  getStatistics().createHistogram(
    new TH2F("time_diff_per_scin",
      "Signals Time Difference per Scintillator ID",
      200, -20000.0, 20000.0,
      192, 1.0, 193.0
    )
  );

  getStatistics().createHistogram(
    new TH2F("hit_pos_per_scin",
      "Hit Position per Scintillator ID",
      200, -150.0, 150.0,
      192, 1.0, 193.0
    )
  );

  if (isOptionSet(fParams.getOptions(), fTimeWindowWidthParamKey)) {
    kTimeWindowWidth = getOptionAsFloat(fParams.getOptions(), fTimeWindowWidthParamKey);
  }
  
  INFO("Hit finding started.");
  
  return true;
}

bool HitFinder::exec()
{
  if(auto & timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    uint n = timeWindow->getNumberOfEvents();
    for(uint i=0;i<n;++i){
      fillSignalsMap(dynamic_cast<const JPetPhysSignal&>(timeWindow->operator[](i)));
    }
    
    vector<JPetHit> hits = HitTools.createHits(
					       getStatistics(),
					       fAllSignalsInTimeWindow,
					       kTimeWindowWidth,
					       fVelocityMap);
    saveHits(hits);
    getStatistics().getHisto1D("hits_per_time_window")->Fill(hits.size());
    fAllSignalsInTimeWindow.clear();
  }else{
    return false;
  }
  return true;
}



bool HitFinder::terminate()
{
  INFO("Hit finding ended.");
  return true;
}


void HitFinder::saveHits(const vector<JPetHit>& hits)
{
	auto sortedHits = JPetAnalysisTools::getHitsOrderedByTime(hits);

	for (const auto & hit : sortedHits) {
	  fOutputEvents->add<JPetHit>(hit);
	}
}

void HitFinder::fillSignalsMap(const JPetPhysSignal signal)
{
	auto scinId = signal.getPM().getScin().getID();
	if (signal.getPM().getSide() == JPetPM::SideA) {
		if (fAllSignalsInTimeWindow.find(scinId) != fAllSignalsInTimeWindow.end()) {
			fAllSignalsInTimeWindow.at(scinId).first.push_back(signal);
		} else {
			std::vector<JPetPhysSignal> sideA = {signal};
			std::vector<JPetPhysSignal> sideB;
			fAllSignalsInTimeWindow.insert(std::make_pair(scinId,
						std::make_pair(sideA, sideB)));
		}
	} else {
		if (fAllSignalsInTimeWindow.find(scinId) != fAllSignalsInTimeWindow.end()) {
			fAllSignalsInTimeWindow.at(scinId).second.push_back(signal);
		} else {
			std::vector<JPetPhysSignal> sideA;
			std::vector<JPetPhysSignal> sideB = {signal};
			fAllSignalsInTimeWindow.insert(std::make_pair(scinId,
						std::make_pair(sideA, sideB)));
		}
	}
}

map<int, vector<double>> HitFinder::readVelocityFile(){

	map<int, vector<double>> velocitiesMap;

	ifstream input;
	input.open("resultsForThresholda.txt");
	if(input.is_open()) INFO("File with velocities for THR A opened.");
	else INFO("File with velocities ERROR.");

	int slot = 0;
	double vel = 0.0, error = 0.0;

	while(!input.eof()){

		input>>slot>>vel>>error;

		vector<double> values;
		values.push_back(vel);
		values.push_back(error);

		velocitiesMap[slot] = values;
	}

	return velocitiesMap;
}
