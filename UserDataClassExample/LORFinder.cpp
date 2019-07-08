/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
 *  @file LORFinder.cpp
 */

#include "LORFinder.h"
#include "../LargeBarrelAnalysis/EventCategorizerTools.h"
#include "JPetLORevent.h"
#include <iostream>
#include <string>

using namespace jpet_options_tools;

using namespace std;

LORFinder::LORFinder(const char *name) : JPetUserTask(name) {}

bool LORFinder::init() {
  INFO("Creation of LOR events started");

  // Set this to the name of your custom data class
  std::string output_class_name = "JPetLORevent";
  fOutputEvents = new JPetTimeWindow(output_class_name.c_str());

  return true;
}

bool LORFinder::exec() {
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow *const>(fEvent)) {

    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto &event =
          dynamic_cast<const JPetEvent &>(timeWindow->operator[](i));

      if (event.getHits().size() == 2 &&
          EventCategorizerTools::checkFor2Gamma(event, getStatistics(), false,
                                                fB2BSlotThetaDiff)) {

        // if the event looks like a 2-gamma one,
        // reconstruct the annihilation point on the LOR
        TVector3 annihilation_point =
            EventCategorizerTools::calculateAnnihilationPoint(
                event.getHits().at(0), event.getHits().at(1));
        // and store it as a JPetLORevent
        JPetLORevent lor_event = event;
        lor_event.setAnnihilationPoint(annihilation_point);
        fOutputEvents->add<JPetLORevent>(lor_event);
      }
    }

  } else {
    return false;
  }
  return true;
}

bool LORFinder::terminate() {
  INFO("Creation of LOR events finished.");
  return true;
}
