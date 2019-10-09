/**
 *  @copyright Copyright 2019 The J-PET Framework Authors. All rights reserved.
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
 *  @file JPetLORevent.cpp
 */

#include "JPetLORevent.h"

ClassImp(JPetLORevent);

JPetLORevent::JPetLORevent() : JPetEvent() { /**/ }

JPetLORevent::JPetLORevent(const JPetEvent &event) : JPetEvent(event) { /**/ }

JPetLORevent::JPetLORevent(const std::vector<JPetHit> &hits,
                           const TVector3 &anh_point, JPetEventType eventType,
                           bool orderedByTime)
    : JPetEvent(hits, eventType, orderedByTime), fAnnihilationPoint(anh_point) {
  /**/
}

void JPetLORevent::Clear(Option_t *) {
  fType = kUnknown;
  fHits.clear();
}

void JPetLORevent::setAnnihilationPoint(double x, double y, double z) {
  setAnnihilationPoint(TVector3(x, y, z));
}

void JPetLORevent::setAnnihilationPoint(const TVector3 &point) {
  fAnnihilationPoint = point;
}

const TVector3 &JPetLORevent::getAnnihilationPoint() const {
  return fAnnihilationPoint;
}
