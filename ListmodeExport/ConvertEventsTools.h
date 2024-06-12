/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
 *  @file ConvertEventsTools.h
 */

#ifndef CONVERTEVENTSTOOLS_H
#define CONVERTEVENTSTOOLS_H

#include <TVector3.h>
#include <string>
#include <vector>

class ConvertEventsTools
{
public:
  static std::vector<TVector3> readLUTFile(const std::string& lutFileName, int numberOfCrystals);

  static uint32_t getCrystalID(const TVector3& hitPos, std::vector<TVector3>& crystals, double maxZSize);
};
#endif /* !CONVERTEVENTSTOOLS_H */
