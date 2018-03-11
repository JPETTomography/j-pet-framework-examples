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

enum kUndefined { point = 999, tof = 9999 };

struct Point3D
{
  double x=0;
  double y=0;
  double z=0;
};

class EventCategorizerTools
{
public:
  static bool checkFor2Gamma(const JPetEvent& event);
  static bool checkFor3Gamma(const JPetEvent& event);
  static bool checkForPrompt(const JPetEvent& event);
  static bool checkForScatter(const JPetEvent& event);
  static double calculateTOF(const JPetHit& firstHit, const JPetHit& latterHit);
  static Point3D calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& latterHit);
};

#endif /*  !EVENTCATEGORIZERTOOLS_H */
