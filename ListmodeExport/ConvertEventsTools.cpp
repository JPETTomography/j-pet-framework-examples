/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
 *  @file ConvertEventsTools.cpp
 */

#include "ConvertEventsTools.h"
#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include "JPetLoggerInclude.h"
#include <cmath>
#include <tuple>

using namespace std;

vector<TVector3> ConvertEventsTools::readLUTFile(const string& lutFileName, int numberOfCrystals)
{
  vector<TVector3> crystals;

  FILE* lutFile = fopen(lutFileName.c_str(), "rb");
  if (lutFile == NULL)
  {
    ERROR("Failed to open " << lutFileName << " file, returning empty vector!");
    return crystals;
  }

  // Read data for each index
  int nb_data_read = 0;
  for (int i = 0; i < numberOfCrystals; i++)
  {
    float x, y, z;
    float skip;
    // Read central crystal position X, then Y and Z in [mm]
    nb_data_read += fread(&x, sizeof(x), 1, lutFile);
    nb_data_read += fread(&y, sizeof(y), 1, lutFile);
    nb_data_read += fread(&z, sizeof(z), 1, lutFile);
    // Read crystal orientation X, then Y and Z
    nb_data_read += fread(&skip, sizeof(skip), 1, lutFile);
    nb_data_read += fread(&skip, sizeof(skip), 1, lutFile);
    nb_data_read += fread(&skip, sizeof(skip), 1, lutFile);

    // Setting the vector in [cm]
    TVector3 crystal(x / 10.0, y / 10.0, z / 10.0);
    crystals.push_back(crystal);
  }

  // Close file
  fclose(lutFile);

  // Check the consistency of read file
  if (nb_data_read != numberOfCrystals * 6)
  {
    ERROR("Failed to read correct number of crystals from LUT file! Returning empty vector.");
    ERROR("Read: " << nb_data_read << " Expected: " << numberOfCrystals * 6);
    vector<TVector3> empty;
    return empty;
  }

  INFO(Form("Number of crystals read from the LUT file: %d", nb_data_read / 6));

  return crystals;
}

/**
 * This function assigns a hit to a 'crystal' in CASToR configuration
 * by finding which object is the closest in the position to the hit.
 *

 checks every crystal from LUT file and calculates
 * distance to hit position. This function assumes that lowest distance
 * is crystal that we are looking for.
 *
 * TODO: add maximal allowed distance
 * TODO: add warning/error when distance is relative big
 * TODO: this function probably could rewritten to be much faster
 * TODO: take orientation of crystal into account
 *
 * @param x X position (in mm) of hit
 * @param y Y position (in mm) of hit
 * @param z Z position (in mm) of hit
 *
 * @return Crystal ID in CASToR LUT file, -1 in case of error
 */
uint32_t ConvertEventsTools::getCrystalID(const TVector3& hitPos, vector<TVector3>& crystals, double maxZSize)
{
  uint32_t closestCrystalID = -1;
  double lowestDistance = 100000.0;

  for (uint32_t id = 0; id < crystals.size(); ++id)
  {
    auto distance = (crystals.at(id) - hitPos).Mag();

    if (distance < lowestDistance)
    {
      lowestDistance = distance;
      closestCrystalID = id;
    }
  }

  // The found distance cannot be larger than the size of the 'crystals' in the
  // configuration - in this case it means that there is no suitable crystal
  // to be associated with this hit - returning error value -1
  if (lowestDistance > maxZSize)
  {
    return -1;
  }
  else
  {
    return closestCrystalID;
  }
}
