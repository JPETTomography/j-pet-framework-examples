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
 *  @file ModuleD.h
 */

#ifndef MODULED_H
#define MODULED_H

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

class ModuleD:public JPetTask {

    public:
        ModuleD(const char * name, const char * description);
        virtual ~ModuleD();
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;

    protected:
        bool isFirstSignalSet = false;
        int timeWindowIndex;

        std::map <int, std::pair <std::vector<JPetPhysSignal>, std::vector<JPetPhysSignal>> > fAllSignalsInTimeWindow;
        // Map od all signals within a single time window.
        // It has a structure of < ScintillatorID <SignalsOnSideA, SignalsOnSideB> >

        void fillSignalsMap(JPetPhysSignal signal);
        std::vector<JPetHit> createHits(std::map <int, std::pair <std::vector<JPetPhysSignal>, std::vector<JPetPhysSignal>> > fAllSignalsInTimeWindow, const double kTimeWindow);
        void saveHits(const std::vector<JPetHit>&hits);

        JPetWriter* fWriter;
        const double kTimeWindow = 50000; /* in ps -> 50ns*/
};

#endif /*  !MODULED_H */
