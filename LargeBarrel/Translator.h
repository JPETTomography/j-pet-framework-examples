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
 *  @file Translator.h
 */

#ifndef Translator_H
#define Translator_H


#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetTOMBChannel/JPetTOMBChannel.h>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for translating HLD data into J-PET data structure.
 *
 * This module is meant to be first in data analysis chain. As an input it takes unpacked HLD file
 * and translates it's content into JPetTimeWindow data structure for further analysis.
 *
 */
class Translator: public JPetTask{
    public:
        Translator(const char * name, const char * description);
        virtual ~Translator();
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;
        virtual void setParamManager(JPetParamManager* paramManager)override;
        const JPetParamBank& getParamBank()const;

    protected:
        void saveTimeWindow( JPetTimeWindow slot);
        JPetSigCh generateSigCh(const JPetTOMBChannel & channel, JPetSigCh::EdgeType edge) const;
        JPetWriter* fWriter = 0;
        JPetParamManager* fParamManager = 0;
        long long int fCurrEventNumber = -1;

        // DAQ time window width. Time of signals should be within this time window
        const double kDAQTimeWindowWidth = 1.e6;

        const double kMaxTime = 0.;     //
        const double kMinTime = -kDAQTimeWindowWidth;  //

};

#endif /*  !Translator_H */
