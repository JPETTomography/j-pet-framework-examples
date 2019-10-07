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
 *  @file JPetLORevent.h
 *  @brief Example of a user-defined data class to store 2-gamma annihilations
 */

#ifndef JPETLOREVENT_H
#define JPETLOREVENT_H

#include <JPetEvent/JPetEvent.h>
#include <TVector3.h>

/**
 * @brief Example of a user-defined data class representing a 2-gamma event
 * derived from JPetEvent and including reconsttructed 2-gamma annihilation
 * point
 *
 */

class JPetLORevent : public JPetEvent {

public:
  JPetLORevent();
  JPetLORevent(const JPetEvent &event);
  JPetLORevent(const std::vector<JPetHit> &hits, const TVector3 &anh_point,
               JPetEventType eventType = JPetEventType::kUnknown,
               bool orderedByTime = true);

  void Clear(Option_t *opt = "");

  void setAnnihilationPoint(double x, double y, double z);
  void setAnnihilationPoint(const TVector3 &point);

  const TVector3 &getAnnihilationPoint() const;

protected:
  TVector3 fAnnihilationPoint;

  ClassDef(JPetLORevent, 1);
};
#endif /*  !JPETLOREVENT_H */
