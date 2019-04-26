/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file SDARecoChargeCalc.cpp
 */

#include "JPetRecoChargeCalc/SDARecoChargeCalc.h"
#include "JPetRecoSignalTools/JPetRecoSignalTools.h"

SDARecoChargeCalc::SDARecoChargeCalc(const char *name)
    : JPetUserTask(name), fBadSignals(0), fCurrentEventNumber(0) {}

SDARecoChargeCalc::~SDARecoChargeCalc() {}

bool SDARecoChargeCalc::init() {
  DEBUG("SDARecoChargeCalc::init()");
  fOutputEvents = new JPetTimeWindow("JPetRecoSignal");
  fBadSignals = 0;
  return true;
}

bool SDARecoChargeCalc::exec() {
  DEBUG("SDARecoChargeCalc::exec()");
  if (auto oldTimeWindow = dynamic_cast<const JPetTimeWindow *const>(fEvent)) {
    auto n = oldTimeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {
      auto signal =
          dynamic_cast<const JPetRecoSignal &>(oldTimeWindow->operator[](i));
      double charge =
          JPetRecoSignalTools::calculateAreaFromStartingIndex(signal);
      if (charge == JPetRecoSignalTools::ERRORS::badCharge) {
        WARNING(
            Form("Something went wrong when calculating charge for event: %d",
                 fCurrentEventNumber));
        JPetRecoSignalTools::saveBadSignalIntoRootFile(signal, fBadSignals,
                                                       "badCharges.root");
        fBadSignals++;
      } else {
        auto signalWithCharge = signal;
        signalWithCharge.setCharge(charge);
        fOutputEvents->add<JPetRecoSignal>(signalWithCharge);
      }
      fCurrentEventNumber++;
    }
  } else {
    return false;
  }

  return true;
}

bool SDARecoChargeCalc::terminate() {
  DEBUG("SDARecoChargeCalc::terminate()");
  int fEventNb = fCurrentEventNumber;
  double goodPercent = (fEventNb - fBadSignals) * 100.0 / fEventNb;
  INFO(Form("Charge` calculation complete \nAmount of bad signals: %d \n %f %% "
            "of data is good",
            fBadSignals, goodPercent));
  return true;
}
