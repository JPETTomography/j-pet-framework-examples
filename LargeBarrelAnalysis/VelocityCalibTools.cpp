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
 *  @file VelocityCalibTools.cpp
 */

#include "VelocityCalibTools.h"
#include "JPetLoggerInclude.h"

double VelocityCalibTools::getVelocity(const VelocityCalibTools::scintillatorVelocities& velocities, const unsigned int scintID)
{
  if( velocities.find(scintID) == velocities.end() )
  {
    WARNING("No velocity calibration available for this channel");
    return 0.0;
  }
  else
    return velocities.at(scintID);
}

VelocityCalibTools::scintillatorVelocities VelocityCalibTools::loadVelocities(const std::string& velocityFile)
{
  scintillatorVelocities velocities;
  try
  {
    std::ifstream inputFile;
    inputFile.open( velocityFile.c_str() );
    unsigned int scintID;
    double velocity;
    while( inputFile >> scintID >> velocity )
      velocities.insert(std::make_pair(scintID, velocity));
    inputFile.close();
  } catch (const std::runtime_error& error)
  {
    std::string msg = "Error opening velocity calibration file: " + std::string(error.what()); 
    ERROR(msg);
  }
  
  
  return velocities;
}
