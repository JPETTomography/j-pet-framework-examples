/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizerTools.h
 */

#ifndef _EVENTCATEGORIZERTOOLS_H_
#define _EVENTCATEGORIZERTOOLS_H_

#include <JPetHit/JPetHit.h>

class Point3D
{
public:
  Point3D() : x(0), y(0), z(0) {}
  Point3D(double initialValue) : x(initialValue), y(initialValue), z(initialValue) {}
  double x;
  double y;
  double z;
};

class EventCategorizerTools
{
public:

  static constexpr float kUndefinedPoint = 999;
  static constexpr float kUndefinedTof = 9999;

  static double calculateTOF(const JPetHit& firstHit, const JPetHit& latterHit);
  static Point3D calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& latterHit);



private:
  EventCategorizerTools() = delete;
};

#endif /*  !EVENTCATEGORIZERTOOLS_H */
