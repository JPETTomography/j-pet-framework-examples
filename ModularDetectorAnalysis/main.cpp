/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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

#include "EventCategorizer.h"
#include "EventFinder.h"
#include "HitFinder.h"
#include "SignalFinder.h"
#include "SignalTransformer.h"
#include "TimeWindowCreator.h"

#include <JPetManager/JPetManager.h>
using namespace std;

int main(int argc, const char* argv[])
{
  try
  {
    JPetManager& manager = JPetManager::getManager();

    manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
    manager.registerTask<SignalFinder>("SignalFinder");
    manager.registerTask<SignalTransformer>("SignalTransformer");
    manager.registerTask<HitFinder>("HitFinder");
    manager.registerTask<EventFinder>("EventFinder");
    manager.registerTask<EventCategorizer>("EventCategorizer");

    manager.useTask("TimeWindowCreator", "hld", "tslot");
    manager.useTask("SignalFinder", "tslot", "pm.sig");
    manager.useTask("SignalTransformer", "pm.sig", "mtx.sig");
    manager.useTask("HitFinder", "mtx.sig", "hits");
    manager.useTask("EventFinder", "hits", "unk.evt");
    manager.useTask("EventCategorizer", "unk.evt", "cat.evt");

    manager.run(argc, argv);
  }
  catch (const std::exception& except)
  {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
