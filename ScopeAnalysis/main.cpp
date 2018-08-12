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

#include <JPetRecoDrawAllCharges/SDARecoDrawAllCharges.h>
#include <JPetRecoAmplitudeCalc/SDARecoAmplitudeCalc.h>
#include <JPetMakePhysSignal/SDAMakePhysSignals.h>
#include <JPetRecoOffsetCalc/SDARecoOffsetsCalc.h>
#include <JPetRecoChargeCalc/SDARecoChargeCalc.h>
#include <JPetMatchLORs/SDAMatchLORs.h>
#include <JPetMatchHits/SDAMatchHits.h>
#include <JPetManager/JPetManager.h>

using namespace std;

int main(int argc, const char* argv[])
{
  try {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<SDARecoOffsetsCalc>("SDARecoOffsetsCalc");
    manager.registerTask<SDARecoChargeCalc>("SDARecoChargeCalc");
    manager.registerTask<SDARecoAmplitudeCalc>("SDARecoAmplitudeCalc");
    manager.registerTask<SDARecoDrawAllCharges>("SDARecoDrawAllCharges");
    manager.registerTask<SDAMakePhysSignals>("SDAMakePhysSignals");
    manager.registerTask<SDAMatchHits>("SDAMatchHits");
    manager.registerTask<SDAMatchLORs>("SDAMatchLORs");

    manager.useTask("SDARecoOffsetsCalc", "reco.sig", "reco.sig.offsets");
    manager.useTask("SDARecoChargeCalc", "reco.sig.offsets", "reco.sig.offsets.charges");
    manager.useTask("SDARecoAmplitudeCalc", "reco.sig.offsets.charges", "reco.sig.offsets.charges.ampl");
    manager.useTask("SDARecoDrawAllCharges", "reco.sig.offsets.charges.ampl", "reco.sig.offsets.charges.ampl.draw");
    manager.useTask("SDAMakePhysSignals", "reco.sig.offsets.charges.ampl", "phys.sig");
    manager.useTask("SDAMatchHits", "phys.sig", "phys.hit");
    manager.useTask("SDAMatchLORs", "phys.hit", "phys.lor");

    manager.run(argc, argv);
  } catch (const std::exception& except) {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
