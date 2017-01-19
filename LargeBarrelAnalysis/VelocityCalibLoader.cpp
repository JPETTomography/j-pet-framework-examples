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
 *  @file VelocityCalibLoader.cpp
 */

#include "VelocityCalibLoader.h"
#include "VelocityCalibTools.h"

VelocityCalibLoader::VelocityCalibLoader(const char* name, const char* description): JPetTask(name, description)
{
  /**/
}

VelocityCalibLoader::~VelocityCalibLoader()
{

}

void VelocityCalibLoader::init(const JPetTaskInterface::Options& opts)
{
  fVelocityCalibration = VelocityCalibTools::loadVelocities("velocityCalibTest.txt");
}

void VelocityCalibLoader::exec()
{
  if( auto oldHit = dynamic_cast<const JPetHit* const> (getEvent())) {
    auto correctedHit = createCorrectedHit(oldHit);
    /*This assumes that you have proper time callibration and filled timeDiff field*/
       
    assert(fWriter);
    fWriter->write(correctedHit);
  }
}


void VelocityCalibLoader::terminate()
{
}

void VelocityCalibLoader::setWriter(JPetWriter* writer)
{
  fWriter = writer;
}

JPetHit VelocityCalibLoader::createCorrectedHit(const JPetHit& oldHit)
{
  JPetHit hitWithInteractionPoint;
  hitWithInteractionPoint.setPosY( oldHit->getPosY() );
  hitWithInteractionPoint.setPosZ( oldHit->getPosZ() );
    
  hitWithInteractionPoint.setBarrelSlot( const_cast<JPetBarrelSlot&>(oldHit->getBarrelSlot()) );
  hitWithInteractionPoint.setScinID( oldHit->getScinID() );
  hitWithInteractionPoint.setScintillator( const_cast<JPetScin&> (oldHit->getScintillator()) );    
    
  hitWithInteractionPoint.setEnergy( oldHit->getEnergy());
  hitWithInteractionPoint.setQualityOfEnergy( oldHit->getQualityOfEnergy() );
  hitWithInteractionPoint.setTime( oldHit->getTime() );
  hitWithInteractionPoint.setQualityOfTime( oldHit->getQualityOfTime() );
  hitWithInteractionPoint.setTimeDiff( oldHit->getTimeDiff() );
  hitWithInteractionPoint.setQualityOfTimeDiff( oldHit->getQualityOfTimeDiff() );
    
  hitWithInteractionPoint.setSignalA( const_cast<JPetPhysSignal&>(oldHit->getSignalA()) );
  hitWithInteractionPoint.setSignalB( const_cast<JPetPhysSignal&>(oldHit->getSignalB()) );
  
  if( oldHit->getTimeDiff() != 0 )
  {
      hitWithInteractionPoint.setPosZ( oldHit->getTimeDiff() / 1000.0 * VelocityCalibTools::getVelocity(fVelocityCalibration, oldHit->getScintillator().getID()) / 2.0 ) ;
      return hitWithInteractionPoint;
  }
  else
  {
      WARNING("No time difference set for hit, it will be ommitted");
      return oldHit;
  }  
  
}
