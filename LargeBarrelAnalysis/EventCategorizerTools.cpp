/**
 *  @copyright Copyright 2017 The J-PET Framework Authors. All rights reserved.
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
 *  @file EventCategorizerTools.cpp
 */

#include "EventCategorizerTools.h"

Point3D EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& firstHit, const JPetHit& latterHit)
{
  Point3D annihilationPoint(EventCategorizerTools::kUndefinedPoint);

  Point3D firstHitPoint;
  firstHitPoint.x = firstHit.getPosX();
  firstHitPoint.y = firstHit.getPosY();
  firstHitPoint.z = firstHit.getPosZ();

  Point3D latterHitPoint;
  latterHitPoint.x = latterHit.getPosX();
  latterHitPoint.y = latterHit.getPosY();
  latterHitPoint.z = latterHit.getPosZ();

  Point3D middleOfLOR;
  middleOfLOR.x = ( firstHitPoint.x + latterHitPoint.x ) / 2.0;
  middleOfLOR.y = ( firstHitPoint.y + latterHitPoint.y ) / 2.0;
  middleOfLOR.z = ( firstHitPoint.z + latterHitPoint.z ) / 2.0;

  double LORlength = sqrt(pow(( firstHitPoint.x - latterHitPoint.x), 2) +
                          pow((firstHitPoint.y - latterHitPoint.y), 2) +
                          pow((firstHitPoint.z - latterHitPoint.z), 2) );

  double versorOnLOR_x, versorOnLOR_y, versorOnLOR_z;  //towards first hit
  versorOnLOR_x =  fabs( (latterHitPoint.x - firstHitPoint.x) / LORlength );
  versorOnLOR_y = fabs( (latterHitPoint.y - firstHitPoint.y) / LORlength );
  versorOnLOR_z = fabs( (latterHitPoint.z - firstHitPoint.z) / LORlength );

  double velocity = 30.0; //cm/ns
  velocity /= 1000; //cm/ps

  annihilationPoint.x = middleOfLOR.x - versorOnLOR_x * calculateTOF(firstHit, latterHit) * velocity;
  annihilationPoint.y = middleOfLOR.y - versorOnLOR_y * calculateTOF(firstHit, latterHit) * velocity;
  annihilationPoint.z = middleOfLOR.z - versorOnLOR_z * calculateTOF(firstHit, latterHit) * velocity;

  return annihilationPoint;
}

double EventCategorizerTools::calculateTOF(const JPetHit& firstHit, const JPetHit& latterHit)
{
  double TOF = EventCategorizerTools::kUndefinedTof;
  if ( firstHit.getTime() > latterHit.getTime() ) {
    ERROR("First hit time should be earlier than later hit");
    return TOF;
  }
  TOF = firstHit.getTime() - latterHit.getTime();

  double firstHitSlotAngle = firstHit.getBarrelSlot().getTheta();
  double latterHitSlotAngle = latterHit.getBarrelSlot().getTheta();

  if ( firstHitSlotAngle < latterHitSlotAngle )
    return TOF;
  else
    return -1.0 * TOF;

  return TOF;
}
