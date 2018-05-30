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

#include <JPetManager/JPetManager.h>
#include "TimeWindowCreator.h"
#include "SignalFinder.h"
#include "SignalTransformer.h"
#include "HitFinder.h"
#include "EventFinder.h"
#include "EventCategorizer.h"
#include "ImageReco.h"

using namespace std;

int main(int argc, const char* argv[])
{

  JPetManager& manager = JPetManager::getManager();

  manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
  manager.registerTask<SignalFinder>("SignalFinder");
  manager.registerTask<SignalTransformer>("SignalTransformer");
  manager.registerTask<HitFinder>("HitFinder");
  manager.registerTask<EventFinder>("EventFinder");
  manager.registerTask<EventCategorizer>("EventCategorizer");
  manager.registerTask<ImageReco>("ImageReco");

  manager.useTask("TimeWindowCreator", "hld", "tslot.calib");
  manager.useTask("SignalFinder", "tslot.calib", "raw.sig");
  manager.useTask("SignalTransformer", "raw.sig", "phys.sig");
  manager.useTask("HitFinder", "phys.sig", "hits");
  manager.useTask("EventFinder", "hits", "unk.evt");
  manager.useTask("EventCategorizer", "unk.evt", "cat.evt");
  manager.useTask("ImageReco", "unk.evt", "reco");

  manager.run(argc, argv);
}
