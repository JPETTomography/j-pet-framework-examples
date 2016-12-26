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
 *  @file HitFinder.h
 */

#ifndef HitFinder_H
#define HitFinder_H

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>

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
class HitFinder:public JPetTask {

    public:
        HitFinder(const char * name, const char * description);
        virtual ~HitFinder();
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;

    protected:

        /**
         * Map od all signals within a single DAQ time window. It has a structure
         * of < ScintillatorID <SignalsOnSideA, SignalsOnSideB> > Wider
         * described in class descrption */
        typedef std::map <int, std::pair <std::vector<JPetPhysSignal>, std::vector<JPetPhysSignal>> > SignalsContainer;

        int DAQTimeWindowIndex = -1; /// Index which defines a given DAQ time window (defined at the hardware level).

        SignalsContainer fAllSignalsInTimeWindow;


        void fillSignalsMap(JPetPhysSignal signal);
        std::vector<JPetHit> createHits(const SignalsContainer& fAllSignalsInTimeWindow, const double kTimeDifferenceWindow);
        void saveHits(const std::vector<JPetHit>&hits);

        JPetWriter* fWriter = 0;
        const double kTimeWindowWidth = 50000; /// in ps -> 50ns. Maximal time difference between signals

};

#endif /*  !HitFinder_H */
