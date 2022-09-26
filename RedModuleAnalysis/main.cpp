/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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

#include "../ModularDetectorAnalysis/EventCategorizer.h"
#include "../ModularDetectorAnalysis/EventFinder.h"
#include "../ModularDetectorAnalysis/SignalFinder.h"
#include "../ModularDetectorAnalysis/TimeWindowCreator.h"
// #include "RedModuleEventCategorizer.h"
#include "RedModuleHitFinder.h"
#include "RedModuleSignalTransformer.h"
#include <JPetManager/JPetManager.h>

using namespace std;

int main(int argc, const char* argv[])
{
  try
  {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
    manager.registerTask<SignalFinder>("SignalFinder");
    manager.registerTask<RedModuleSignalTransformer>("RedModuleSignalTransformer");
    manager.registerTask<RedModuleHitFinder>("RedModuleHitFinder");
    manager.registerTask<EventFinder>("EventFinder");
    manager.registerTask<EventCategorizer>("EventCategorizer");
    // manager.registerTask<RedModuleEventCategorizer>("RedModuleEventCategorizer");

    manager.useTask("TimeWindowCreator", "hld", "tslot");
    manager.useTask("SignalFinder", "tslot", "sipm.sig");
    manager.useTask("RedModuleSignalTransformer", "sipm.sig", "mtx.sig");
    manager.useTask("RedModuleHitFinder", "mtx.sig", "hits");
    manager.useTask("EventFinder", "hits", "unk.evt");
    manager.useTask("EventCategorizer", "unk.evt", "cat.evt");
    // manager.useTask("RedModuleEventCategorizer", "unk.evt", "cat.evt");

    manager.run(argc, argv);
  }
  catch (const std::exception& except)
  {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
