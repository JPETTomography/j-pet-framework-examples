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
 *  @file SignalFinder.cpp
 */

using namespace std;

#include <map>
#include <string>
#include <vector>
#include <JPetWriter/JPetWriter.h>
#include "SignalFinderTools.h"
#include "SignalFinder.h"
#include <JPetOptionsTools/JPetOptionsTools.h>

using namespace jpet_options_tools;

SignalFinder::SignalFinder(const char* name)
	: JPetUserTask(name)
{
  //	fSaveControlHistos = saveControlHistos;
}

SignalFinder::~SignalFinder() {}

//SignalFinder init method
bool SignalFinder::init()
{
	INFO("Signal finding started.");
	
	fOutputEvents = new JPetTimeWindow("JPetRawSignal");

	if (isOptionSet(fParams.getOptions(), fEdgeMaxTimeParamKey)) {
	  kSigChEdgeMaxTime = getOptionAsFloat(fParams.getOptions(), fEdgeMaxTimeParamKey);
	}else{
	  WARNING(Form("No value of the %s parameter provided by the user. Using default valu of %lf.", fEdgeMaxTimeParamKey.c_str(), kSigChEdgeMaxTime));
	}

	if (isOptionSet(fParams.getOptions(), fLeadTrailMaxTimeParamKey)) {
	  kSigChLeadTrailMaxTime = getOptionAsFloat(fParams.getOptions(), fLeadTrailMaxTimeParamKey);
	}else{
	  WARNING(Form("No value of the %s parameter provided by the user. Using default valu of %lf.", fLeadTrailMaxTimeParamKey.c_str(), kSigChLeadTrailMaxTime));
	}

	if (fSaveControlHistos) {
		getStatistics().createHistogram(
			new TH1F("remainig_leading_sig_ch_per_thr",
				"Remainig Leading Signal Channels",
				4, 0.5, 4.5));
		getStatistics().createHistogram(
			new TH1F("remainig_trailing_sig_ch_per_thr",
				"Remainig Trailing Signal Channels",
				4, 0.5, 4.5));
	}
	return true;
}

//SignalFinder execution method
bool SignalFinder::exec()
{

	//getting the data from event in apropriate format
	if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {

		//mapping method invocation
		map<int, vector<JPetSigCh>> sigChsPMMap = SignalFinderTools::getSigChsPMMapById(timeWindow);

		//building signals method invocation
		vector<JPetRawSignal> allSignals = SignalFinderTools::buildAllSignals(
    							sigChsPMMap,
    							kNumOfThresholds ,
    							getStatistics(),
    							fSaveControlHistos,
    							kSigChEdgeMaxTime,
    							kSigChLeadTrailMaxTime);

		//saving method invocation
		saveRawSignals(allSignals);

	}else{
	  return false;
	}
	return true;
}

//SignalFinder finish method
bool SignalFinder::terminate()
{
	INFO("Signal finding ended.");
	return true;
}


//saving method
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& sigChVec)
{
	for (auto & sigCh : sigChVec) {
	  fOutputEvents->add<JPetRawSignal>(sigCh);
	}
}

