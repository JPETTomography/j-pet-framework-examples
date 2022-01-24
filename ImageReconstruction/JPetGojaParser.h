/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file JPetGojaParser.h
 */

#ifndef JPETGOJAPARSER_H
#define JPETGOJAPARSER_H

#ifdef __CINT__
// when cint is used instead of compiler, override word is not recognized
// nevertheless it's needed for checking if the structure of project is correct
#define override
#endif

#include "JPetUserTask/JPetUserTask.h"
#include <memory>

/**
 * @brief Module converting Goja input file into JPetEvent
 *
 * Input: ASCII file containing Goja events
 * Output: *.unk.evt.root containing converted events
 *
 * This module defines 1 user option:
 * - "JPetGojaParser_InputFilePath_std::string": defines input path for file containing Goja events (required)
 *
 */

class JPetGojaParser : public JPetUserTask {

public:
  JPetGojaParser(const char* name);
  virtual ~JPetGojaParser() override;
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  void setUpOptions();

  const std::string kInputFilePathKey = "JPetGojaParser_InputFilePath_std::string";

  std::string fInputFilePath = "";
};

#endif /*  !JPETGOJAPARSER_H */
