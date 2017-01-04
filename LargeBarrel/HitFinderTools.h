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
 *  @file HitFinderTools.h
 */

#ifndef HITFINDERTOOLS_H
#define HITFINDERTOOLS_H

#include <JPetHit/JPetHit.h>

#include <vector>

class HitFinderTools
{
    public:

        typedef std::map <int, std::pair <std::vector<JPetPhysSignal>, std::vector<JPetPhysSignal>> > SignalsContainer;
        std::vector<JPetHit> createHits(const SignalsContainer& allSignalsInTimeWindow, const double timeDifferenceWindow);

        const double kTimeWindowWidth = 50000; /// in ps -> 50ns. Maximal time difference between signals

};

#endif /*  !HITFINDERTOOLS_H */
