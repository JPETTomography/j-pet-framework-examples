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
 *  @file ModuleC2.cpp
 */

#include <iostream>
#include <JPetWriter/JPetWriter.h>
#include "ModuleC2.h"

using namespace std;

ModuleC2::ModuleC2(const char * name, const char * description):JPetTask(name, description){}

ModuleC2::~ModuleC2(){}

void ModuleC2::init(const JPetTaskInterface::Options& opts){}

void ModuleC2::exec(){

    //getting the data from event in propriate format *Will be changed*
    if(auto currSignal = dynamic_cast<const JPetRawSignal*const>(getEvent())){

        JPetRecoSignal reconstructedSignal;
        reconstructedSignal.setRawSignal(*currSignal);

        JPetPhysSignal physicalSignal;

        physicalSignal.setRecoSignal(reconstructedSignal);
        physicalSignal.setTime(currSignal -> getTimesVsThresholdNumber(JPetSigCh::Leading).at(1));
        /* Time is set to be the time at first treshold of RawSignal */


    }
}

void ModuleC2::terminate(){}

void ModuleC2::setWriter(JPetWriter* writer){fWriter =writer;}
