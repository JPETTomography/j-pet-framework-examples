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
 *  @file SDAMakePhysSignals.cpp
 */

#include "../../tools/JPetRecoSignalTools/JPetRecoSignalTools.h"
#include "SDAMakePhysSignals.h"

SDAMakePhysSignals::SDAMakePhysSignals(const char* name): JPetUserTask(name){}

SDAMakePhysSignals::~SDAMakePhysSignals(){}

bool SDAMakePhysSignals::init()
{
  fOutputEvents = new JPetTimeWindow("JPetPhysSignal");
  return true;
}

bool SDAMakePhysSignals::exec()
{
  if (auto oldTimeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    auto n = oldTimeWindow->getNumberOfEvents();
    for (uint i = 0; i < n; ++i) {
      auto signal = dynamic_cast<const JPetRecoSignal&>(oldTimeWindow->operator[](i));
      JPetPhysSignal physSignal;
      physSignal.setRecoSignal(signal);
      // NOTE: This module currently sets number of photoelectrons
      // equal to charge of JPetRecoSignal
      physSignal.setPhe(physSignal.getRecoSignal().getCharge() );
      fOutputEvents->add<JPetPhysSignal>(physSignal);
    }
  } else {
    return false;
  }
  return true;
}

bool SDAMakePhysSignals::terminate()
{
  return true;
}
