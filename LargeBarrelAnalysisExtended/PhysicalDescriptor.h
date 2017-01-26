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
 *  @file PhysicalDescriptor.h
 */

#ifndef PhysicalDescriptor_H
#define PhysicalDescriptor_H

#include <JPetTask/JPetTask.h>
#include <JPetHit/JPetHit.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <JPetRecoSignal/JPetRecoSignal.h>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#   define override
#endif

/**
 * @brief      Module responsible for creating physical quantities that describe a signal.
 *
 * This module takes reconstructed signal (JPetRecoSignal) as an input and characterize
 * the signal by physical properties such as time of arrival or number of hotoelectrons in a whole signal.
 */
class PhysicalDescriptor: public JPetTask {

    public:
        PhysicalDescriptor(const char * name, const char * description);
        virtual ~PhysicalDescriptor();
        virtual void init(const JPetTaskInterface::Options& opts)override;
        virtual void exec()override;
        virtual void terminate()override;
        virtual void setWriter(JPetWriter* writer)override;

    protected:
        JPetWriter* fWriter;
};

#endif /*  !PhysicalDescriptor_H */
