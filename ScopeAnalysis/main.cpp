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
#include <modules/JPetRecoOffsetCalc/SDARecoOffsetsCalc.h>
#include <modules/JPetRecoChargeCalc/SDARecoChargeCalc.h>
#include <modules/JPetRecoAmplitudeCalc/SDARecoAmplitudeCalc.h>
#include <modules/JPetRecoDrawAllCharges/SDARecoDrawAllCharges.h>
#include <modules/JPetMakePhysSignal/SDAMakePhysSignals.h>
#include <modules/JPetMatchHits/SDAMatchHits.h>
#include <modules/JPetMatchLORs/SDAMatchLORs.h>

using namespace std;
int main(int argc, char* argv[])
{
	DB::SERVICES::DBHandler::createDBConnection("../DBConfig/configDB.cfg");
  JPetManager& manager = JPetManager::getManager();
  manager.parseCmdLine(argc, argv);

  manager.registerTask([](){
      return new JPetTaskLoader("reco.sig", "reco.sig.offsets",
				new SDARecoOffsetsCalc("RecoSigOffsetsCalc",
							"Calculate offsets for signals from SDA"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("reco.sig.offsets", "reco.sig.offsets.charges",
  				new SDARecoChargeCalc("RecoSigChargeCalc",
  							"Calculate charges for signals from SDA"));
    });

  manager.registerTask([](){
      return new JPetTaskLoader("reco.sig.offsets.charges", "reco.sig.offsets.charges.ampl",
  				new SDARecoAmplitudeCalc("RecoSigAmplitudeCalc",
						      "Calculate amplitides for signals from SDA"));
    });
  
  manager.registerTask([](){
      return new JPetTaskLoader("reco.sig.offsets.charges.ampl", "reco.sig.offsets.charges.ampl.draw",
  				new SDARecoDrawAllCharges("RecoDrawAllCharges",
							 "Draw the charge spaectra for each photomultiplier"));
    });
  
  manager.registerTask([](){
      return new JPetTaskLoader("reco.sig.offsets.charges.ampl", "phys.sig",
  				new SDAMakePhysSignals("MakePhysSig",
  						       "Transform reco signals to physical signals"));
    });

    manager.registerTask([](){
      return new JPetTaskLoader("phys.sig", "phys.hit",
  				new SDAMatchHits("MatchHits",
  						       "Assemble pairs of phys signals into hits"));
    });

      manager.registerTask([](){
      return new JPetTaskLoader("phys.hit", "phys.lor",
  				new SDAMatchLORs("MatchLORs",
  						       "Assemble pairs of hits into Lines Of Response"));
    });



  manager.run();
}
