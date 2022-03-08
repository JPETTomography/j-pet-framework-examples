#include "ConvertEventsTools.h"
#include "JPetLoggerInclude.h"
#include <tuple>
#include <cmath>

bool ConvertEventsTools::readLUTFile(const std::string& path, int numberOfCrystals, std::vector<std::tuple<float, float, float>> &readedLutGeometry) {
  FILE* LUT_file = fopen(path.c_str(), "rb");
  if (LUT_file==NULL)
  {
    ERROR("Failed to open " << path << " file, aborting!");
    return false;
  }

  // Read data for each index
  int nb_data_read = 0;
  for (int i=0; i<numberOfCrystals; i++)
  {
    float x, y, z;
    float skip;
    // Read central crystal position X, then Y and Z
    nb_data_read += fread(&x,sizeof(x),1,LUT_file);
    nb_data_read += fread(&y,sizeof(y),1,LUT_file);
    nb_data_read += fread(&z,sizeof(z),1,LUT_file);
    // Read crystal orientation X, then Y and Z
    nb_data_read += fread(&skip,sizeof(skip),1,LUT_file);
    nb_data_read += fread(&skip,sizeof(skip),1,LUT_file);
    nb_data_read += fread(&skip,sizeof(skip),1,LUT_file);
    readedLutGeometry.push_back(std::make_tuple(x, y, z));
    
    //    std::cout << "[ " << x << ", " << y << ", " << z << " ]"  << std::endl;
  }

  // Close file
  fclose(LUT_file);
  // Check reading
  if (nb_data_read!=numberOfCrystals*6) {
    ERROR("Failed to read correct number of crystals from lut file!");
    ERROR("Readed: " << nb_data_read << " Expected: " << numberOfCrystals * 6);
    return false;
  }
  return true;
}

/**
 * This function checks every crystal from LUT file and calculates
 * distance to hit position. This function assumes, that lowest distance
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
 * @return ID of hit in CASToR LUT file
 */
uint32_t ConvertEventsTools::getCastorID(float x, float y, float z, const std::vector<std::tuple<float, float, float>> &castorIDs) {
  uint32_t i = 0;
  // TODO: generally doesn't metter, but maybe pick more meaningful staring lowestDistance
  float lowestDistance = 100000;
  uint32_t lowestI = 0;

  for (auto t : castorIDs) {
    float diffX = std::get<0>(t) - x;
    float diffY = std::get<1>(t) - y;
    float diffZ = std::get<2>(t) - z;
    float distance = std::sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

    if (distance < lowestDistance) {
      lowestDistance = distance;
      lowestI = i;
    }
    i++;
  }
  return lowestI;
}
