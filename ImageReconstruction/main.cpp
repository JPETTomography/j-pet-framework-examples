/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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

#include "FilterEvents.h"
#include "ImageReco.h"
#include "JPetManager/JPetManager.h"
#include "MLEMRunner.h"
#include "ReconstructionTask.h"
#include "SinogramCreator.h"
#include "SinogramCreatorMC.h"
#include "SinogramCreatorTOF.h"
using namespace std;

int main(int argc, const char* argv[])
{
  try
  {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<FilterEvents>("FilterEvents");
    manager.registerTask<ImageReco>("ImageReco");
    manager.registerTask<SinogramCreator>("SinogramCreator");
    manager.registerTask<SinogramCreatorMC>("SinogramCreatorMC");
    manager.registerTask<MLEMRunner>("MLEMRunner");
    manager.registerTask<SinogramCreatorTOF>("SinogramCreatorTOF");
    manager.registerTask<ReconstructionTask>("ReconstructionTask");

    // manager.useTask("FilterEvents", "unk.evt", "reco.unk.evt");
    // manager.useTask("MLEMRunner", "reco.unk.evt", "");
    // manager.useTask("ImageReco", "reco.unk.evt", "reco");
    // manager.useTask("SinogramCreator", "reco.unk.evt", "sino");
    // manager.useTask("SinogramCreatorMC", "reco.unk.evt", "sino.mc");
    manager.useTask("SinogramCreatorTOF", "reco.unk.evt", "sino.mc");
    manager.useTask("ReconstructionTask", "sino.mc", "reco.mc");

    manager.run(argc, argv);
  }
  catch (const std::exception& except)
  {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
