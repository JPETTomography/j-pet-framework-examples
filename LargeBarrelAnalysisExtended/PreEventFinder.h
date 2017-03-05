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
 *  @file PreEventFinder.h
 */

#ifndef PREEVENTFINDER_H
#define PREEVENTFINDER_H

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetEvent/JPetEvent.h>
// #include "PreEventFinderTools.h"

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for creating JPetHit from signals on oppositte photomultipliers
 *
 * This module takes all physical signals (JPetPhysSignal) within a single time window (JPetTimeWindow)
 * and creates special type of container - SignalsContainer.
 * For each scintillator with at least one signal this container stores two vectors
 * one for physical signals on photomultiplier on side A and second for signals on side B. Then
 * for each signal on side A it searches for corresponding signal on side B - that is time difference of arrival
 * of those two signals needs to be less then specified time difference (kTimeWindowWidth)
 *
 */
class PreEventFinder: public JPetTask
{

  public:
    PreEventFinder(const char* name, const char* description);
    virtual ~PreEventFinder();
    virtual void init(const JPetTaskInterface::Options& opts)override;
    virtual void exec()override;
    virtual void terminate()override;
    virtual void setWriter(JPetWriter* writer)override;

  protected:
    JPetWriter* fWriter = 0;
    std::vector<JPetHit> fHits;
    const double kSubsequentHitsTimeDiff = 10000; /*ps*/
    std::vector<JPetEvent> CreatePreEvents( std::vector<JPetHit>& Hits, double subsequentHitsTimeDiff);
    void SavePreEvents( std::vector<JPetEvent> PreEventsInTimeWindow);
};

#endif /*  !PREEVENTFinder_H */
