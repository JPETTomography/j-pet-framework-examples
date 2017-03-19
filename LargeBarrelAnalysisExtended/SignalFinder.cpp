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

SignalFinder::SignalFinder(const char* name, const char* description, bool saveControlHistos)
	: JPetTask(name, description)
{
	fSaveControlHistos = saveControlHistos;
}

SignalFinder::~SignalFinder() {}

//SignalFinder init method
void SignalFinder::init(const JPetTaskInterface::Options& opts)
{
	INFO("Signal finding started.");

	if (opts.count(fEdgeMaxTimeParamKey)) {
		kSigChEdgeMaxTime = std::atof(opts.at(fEdgeMaxTimeParamKey).c_str());
	}

	if (opts.count(fLeadTrailMaxTimeParamKey)) {
		kSigChLeadTrailMaxTime = std::atof(opts.at(fLeadTrailMaxTimeParamKey).c_str());
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
}

//SignalFinder execution method
void SignalFinder::exec()
{

	//getting the data from event in apropriate format
	if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(getEvent())) {

		//mapping method invocation
		map<int, vector<JPetSigCh>> sigChsPMMap = SignalFinderTools::getSigChsPMMapById(timeWindow);

		//building signals method invocation
		vector<JPetRawSignal> allSignals = SignalFinderTools::buildAllSignals(
								timeWindow->getIndex(),
    							sigChsPMMap,
    							kNumOfThresholds ,
    							getStatistics(),
    							fSaveControlHistos,
    							kSigChEdgeMaxTime,
    							kSigChLeadTrailMaxTime);

		//saving method invocation
		saveRawSignals(allSignals);

	}
}

//SignalFinder finish method
void SignalFinder::terminate()
{
	INFO("Signal finding ended.");
}


//saving method
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& sigChVec)
{
	assert(fWriter);
	for (const auto & sigCh : sigChVec) {
		fWriter->write(sigCh);
	}
}

void SignalFinder::setWriter(JPetWriter* writer)
{
	fWriter = writer;
}
