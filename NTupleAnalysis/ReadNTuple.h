/**
 *  @copyright Copyright 2025 The J-PET Framework Authors. All rights reserved.
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
 *  @file ReadNTuple.h
 */

#ifndef READNTUPLE_H
#define READNTUPLE_H

#include <JPetEvent/JPetEvent.h>
#include <JPetUserTask/JPetUserTask.h>

#include <map>
#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>

class ReadNTuple : public JPetUserTask
{

public:
  ReadNTuple(const char* name);
  virtual ~ReadNTuple() {}
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void saveEvent(JPetEvent& event);
};

#endif /* !READNTUPLE_H */
