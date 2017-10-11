/**
 *  @copyright Copyright 2016 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file TimeCalibLoader.cpp
 */

#include "TimeCalibLoader.h"
#include "TimeCalibTools.h"
#include "JPetGeomMapping/JPetGeomMapping.h"
#include <JPetParamManager/JPetParamManager.h>
#include <JPetOptionsTools/JPetOptionsTools.h>

using namespace jpet_options_tools;

TimeCalibLoader::TimeCalibLoader(const char* name):
  JPetUserTask(name) {}

TimeCalibLoader::~TimeCalibLoader() {}

bool TimeCalibLoader::init()
{
  fOutputEvents = new JPetTimeWindow("JPetSigCh");

  auto calibFile =  std::string("timeCalib.txt");

  if (isOptionSet(fParams.getOptions(),fConfigFileParamKey)) {
    calibFile = getOptionAsString(fParams.getOptions(),fConfigFileParamKey);
  }else{
    WARNING("No path to the time calibration file was provided in user options.");
  }

  JPetGeomMapping mapper(getParamBank());
  auto tombMap = mapper.getTOMBMapping();
  fTimeCalibration = TimeCalibTools::loadTimeCalibration(calibFile, tombMap);
  if (fTimeCalibration.empty()) {
    ERROR("Time calibration seems to be empty");
  }

  return true;
}

bool TimeCalibLoader::exec()
{
  if (auto oldTimeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {

    auto n = oldTimeWindow->getNumberOfEvents();
    for(uint i=0;i<n;++i){

      JPetSigCh sigCh = dynamic_cast<const JPetSigCh&>(oldTimeWindow->operator[](i));
    /// Calibration time is ns so we should change it to ps, cause all the time is in ps.
      sigCh.setValue(sigCh.getValue() + 1000. * TimeCalibTools::getTimeCalibCorrection(fTimeCalibration, sigCh.getTOMBChannel().getChannel()));
      fOutputEvents->add<JPetSigCh>(sigCh);
    }
  }else{
    return false;
  }
  return true;
}

bool TimeCalibLoader::terminate()
{
  return true;
}

