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
 *  @file TimeWindowCreator.h
 */

#ifndef TimeWindowCreator_H
#define TimeWindowCreator_H

#include <JPetTask/JPetTask.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetTOMBChannel/JPetTOMBChannel.h>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif

/// Task to translate EventIII Unpacker data to JPetTimeWindow.
/// Also, some basic filtering can be done

class TimeWindowCreator: public JPetTask
{
public:
  TimeWindowCreator(const char* name, const char* description);
  virtual ~TimeWindowCreator();
  virtual void init(const JPetTaskInterface::Options& opts) override;
  virtual void exec() override;
  virtual void terminate() override;
  virtual void setParamManager(JPetParamManager* paramManager) override;
  const JPetParamBank& getParamBank() const;

protected:
  JPetSigCh generateSigCh(const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge) const;
  JPetParamManager* fParamManager = nullptr;
  long long int fCurrEventNumber = 0;
  const std::string kMaxTimeParamKey = "TimeWindowCreator_MaxTime";
  const std::string kMinTimeParamKey = "TimeWindowCreator_MinTime";
  double fMaxTime = 0.;
  double fMinTime = -1.e6;
};

#endif /*  !TimeWindowCreator_H */
