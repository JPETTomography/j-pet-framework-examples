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
 *  @file JPetGojaParser.cpp
 */

#include "JPetGojaParser.h"
#include "./JPetOptionsTools/JPetOptionsTools.h"
using namespace jpet_options_tools;

JPetGojaParser::JPetGojaParser(const char* name) : JPetUserTask(name) {}

JPetGojaParser::~JPetGojaParser() {}

bool JPetGojaParser::init() {
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  return true;
}

bool JPetGojaParser::exec() {
  std::ifstream in(fInputFilePath);

  float firstX = 0.f;
  float firstY = 0.f;
  float secondX = 0.f;
  float secondY = 0.f;
  float firstZ = 0.f;
  float secondZ = 0.f;
  float firstT = 0.f;
  float secondT = 0.f;

  float skip = 0.f;

  while (in.peek() != EOF) {

    in >> firstX >> firstY >> firstZ >> firstT >> secondX >> secondY >> secondZ >> secondT >> skip >> skip >> skip >> skip >> skip >> skip >> skip >>
        skip;

    JPetHit h1;
    h1.setPos(firstX, firstY, firstZ);
    h1.setTime(firstT);
    JPetHit h2;
    h2.setPos(secondX, secondY, secondZ);
    h2.setTime(secondT);
    std::vector<JPetHit> hits = {h1, h2};
    fOutputEvents->add(JPetEvent(hits));
  }

  return true;
}

bool JPetGojaParser::terminate() { return true; }

void JPetGojaParser::setUpOptions() {
  auto opts = getOptions();
  if (isOptionSet(opts, kInputFilePathKey)) {
    fInputFilePath = getOptionAsString(opts, kInputFilePathKey);
  }
}
