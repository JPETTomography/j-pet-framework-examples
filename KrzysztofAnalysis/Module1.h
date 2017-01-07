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
 *  @file TaskA.h
 */

#ifndef MODULE1_H
#define MODULE1_H

#include <vector>
#include <JPetTask/JPetTask.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetParamBank/JPetParamBank.h>
#include <JPetParamManager/JPetParamManager.h>
#include <JPetTOMBChannel/JPetTOMBChannel.h>

class JPetWriter;
#ifdef __CINT__
#define override
#endif
class Module1: public JPetTask{

public:
  Module1(const char * name, const char * description);
  virtual ~Module1();
  virtual void init(const JPetTaskInterface::Options& opts) override;
  virtual void exec() override;
  virtual void terminate() override;
  virtual void setWriter(JPetWriter* writer) override;
  virtual void setParamManager(JPetParamManager* paramManager) override;
  const JPetParamBank& getParamBank() const;

protected:
  void saveTimeWindow( JPetTimeWindow slot);
  JPetSigCh generateSigCh(const JPetTOMBChannel & channel, JPetSigCh::EdgeType edge) const;
  JPetWriter* fWriter;
  JPetParamManager* fParamManager;
  long long int fCurrEventNumber;
  const double kMaxTime = 0.;
  const double kMinTime = -666700.;
};
#endif 
/*  !MODULE1_H */
