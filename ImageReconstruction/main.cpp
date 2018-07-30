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

#include "ImageReco.h"
#include "JPetManager/JPetManager.h"
#include "MLEMRunner.h"
#include "SinogramCreator.h"
#include "SinogramCreatorMC.h"
using namespace std;

int main(int argc, const char* argv[]) {
  try {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<ImageReco>("ImageReco");
    manager.registerTask<SinogramCreator>("SinogramCreator");
    manager.registerTask<SinogramCreatorMC>("SinogramCreatorMC");
    manager.registerTask<MLEMRunner>("MLEMRunner");

    manager.useTask("MLEMRunner", "reco.unk.evt", "");
    // manager.useTask("ImageReco", "unk.evt", "reco");
    // manager.useTask("SinogramCreator", "unk.evt", "sino");
    // manager.useTask("SinogramCreatorMC", "unk.evt", "sino.mc");

    manager.run(argc, argv);
  } catch (const std::exception& except) {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
