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

#include "../ModularDetectorAnalysis/TimeWindowCreator.h"
#include "../ModularDetectorAnalysis/SignalTransformer.h"
#include "../ModularDetectorAnalysis/SignalFinder.h"
#include "../ModularDetectorAnalysis/HitFinder.h"
#include <JPetManager/JPetManager.h>
#include "TimeCalibration.h"
using namespace std;

int main(int argc, const char* argv[]) {
  try {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
    manager.registerTask<SignalFinder>("SignalFinder");
    manager.registerTask<SignalTransformer>("SignalTransformer");
    manager.registerTask<HitFinder>("HitFinder");
    manager.registerTask<TimeCalibration>("TimeCalibration");

    manager.useTask("TimeWindowCreator", "hld", "tslot.calib");
    manager.useTask("SignalFinder", "tslot.calib", "raw.sig");
    manager.useTask("SignalTransformer", "raw.sig", "phys.sig");
    manager.useTask("HitFinder", "phys.sig", "hits");
    manager.useTask("TimeCalibration", "hits", "calib");

    manager.run(argc, argv);
  } catch (const std::exception& except) {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
