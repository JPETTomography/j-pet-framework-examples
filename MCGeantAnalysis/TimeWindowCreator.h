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

#include <JPetUserTask/JPetUserTask.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetTOMBChannel/JPetTOMBChannel.h>
#include <set>

class JPetWriter;

#ifdef __CINT__
//when cint is used instead of compiler, override word is not recognized
//nevertheless it's needed for checking if the structure of project is correct
#	define override
#endif

/// Task to translate EventIII Unpacker data to JPetTimeWindow.
/// Also, some basic filtering can be done

class TimeWindowCreator: public JPetUserTask
{
public:
  TimeWindowCreator(const char* name);
  virtual ~TimeWindowCreator();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  bool filter(const JPetTOMBChannel& channel) const;
  JPetSigCh generateSigCh(const JPetTOMBChannel& channel, JPetSigCh::EdgeType edge) const;
  long long int fCurrEventNumber = 0;
  const std::string kMaxTimeParamKey = "TimeWindowCreator_MaxTime_double";
  const std::string kMinTimeParamKey = "TimeWindowCreator_MinTime_double";
  const std::string kMainStripKey = "TimeWindowCreator_MainStrip_int";
  std::pair<int,int> fMainStrip;
  bool fMainStripSet = false;
  std::set<int> fAllowedChannels;
  double fMaxTime = 0.;
  double fMinTime = -1.e6;
};

#endif /*  !TimeWindowCreator_H */
