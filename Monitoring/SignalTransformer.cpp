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
 *  @file SignalTransformer.cpp
 */

#include "SignalTransformer.h"
#include "JPetWriter/JPetWriter.h"

SignalTransformer::SignalTransformer(const char* name, const char* description):
	JPetTask(name, description) { }

void SignalTransformer::init(const JPetTaskInterface::Options& opts)
{
	  INFO("Signal transforming started: Raw to Reco and Phys");
	  
	  fOutputEvents = new JPetTimeWindow("JPetPhysSignal");
}


void SignalTransformer::exec()
{
  if(auto & timeWindow = dynamic_cast<const JPetTimeWindow* const>(getEvent())) {

    uint n = timeWindow->getNumberOfEvents();

    for(uint i=0;i<n;++i){
      const JPetRawSignal & currSignal = dynamic_cast<const JPetRawSignal&>(timeWindow->operator[](i));

      //Make Reco Signal from Raw Signal
      auto recoSignal = createRecoSignal(currSignal);

      //Make Phys Signal from Reco Signal and save
      auto physSignal = createPhysSignal(recoSignal);
      fOutputEvents->add<JPetPhysSignal>(physSignal);
    }    
  }

}

void SignalTransformer::terminate()
{
	  INFO("Signal transforming finished");
}

JPetRecoSignal SignalTransformer::createRecoSignal(const JPetRawSignal& rawSignal)
{
	JPetRecoSignal recoSignal;

	//reading threshold times by threshold number
	//from Leading and Trailing edge
	std::map<int,double> leadingPoints = rawSignal.getTimesVsThresholdNumber(JPetSigCh::Leading);
	std::map<int,double> trailingPoints = rawSignal.getTimesVsThresholdNumber(JPetSigCh::Trailing);

	//finding TOT for every threshold 1-4
	std::vector<double> tots;
	for(int i=1;i<5;i++){
		auto leadSearch = leadingPoints.find(i);
		auto trailSearch = trailingPoints.find(i);
		if (leadSearch != leadingPoints.end()
			&& trailSearch != trailingPoints.end())
				tots.push_back(trailSearch->second - leadSearch->second);
	}

	//setting charge of Reco Signal equal to TOT on first threshold
	if(tots.size()!=0){
		recoSignal.setCharge(tots.at(0));
	}else{
		recoSignal.setCharge(-1.0);
	}

	//set the rest of Reco Signal properties to -1.0.
	recoSignal.setDelay(-1.0);
	recoSignal.setOffset(-1.0);
	recoSignal.setAmplitude(-1.0);

	//store the original Raw Signal in the RecoSignal as a processing history
	recoSignal.setRawSignal(rawSignal);
	return recoSignal;
}

JPetPhysSignal SignalTransformer::createPhysSignal(const JPetRecoSignal& recoSignal)
{
	JPetPhysSignal physSignal;

	//use the values from Reco Signal to set the physical properties of signal
	//here is an example - replace with correct method
	physSignal.setPhe(recoSignal.getCharge()*1.0+0.0);
	physSignal.setQualityOfPhe(0.0);

	//Set tmie of Physical Signal as a time of Signal at First Threshold
	//This should be changed to more resonable
	std::map<int,double> leadingPointsMap = recoSignal
				.getRawSignal()
				.getTimesVsThresholdNumber(JPetSigCh::Leading);
	double time = leadingPointsMap.begin()->second;
	physSignal.setTime(time);
	physSignal.setQualityOfTime(0.0);

	//store the original Reco Signal in the Phys Signal as a processing history
	physSignal.setRecoSignal(recoSignal);
	return physSignal;
}
