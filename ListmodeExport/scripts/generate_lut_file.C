/**
 *  @copyright Copyright 2024 The J-PET Framework Authors. All rights reserved.
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
 *  @file generate_lut_file.C
 *
 *  @brief Reads the json configuration of the detector, that follows Frameworks
 *  parameter data structure, and outputs a binary data file that describes
 *  crystal geometry accorinding to CASToR format
 *
 *  Basic usage:
 *  root> .L generate_lut_file.C
 *  root> generate_lut_file("detector_configuration.json", "output_file.lut)
 *  -- this will produce file "output_file.lut" with the results.
 */

#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <TDirectory.h>
#include <TFile.h>

#include <fstream>
#include <iostream>
#include <vector>

namespace bpt = boost::property_tree;

// Scintillators are to be divided into separate detectors ("crystals")
// of the same sizes (dimensions in [cm])
double fCrystalSizeZ = 0.25;

void generate_lut_file(std::string confFileName, int setupID = 38, std::string lutFileName = "jpetDector.lut")
{
  bpt::ptree tree;
  ifstream file(confFileName.c_str());
  if (file.good())
  {
    bpt::read_json(confFileName, tree);
  }

  // Couter of the final number of the crystals
  // int arraySize = 0;
  vector<double> output;

  BOOST_FOREACH (boost::property_tree::ptree::value_type& v, tree.get_child(to_string(setupID) + ".scin"))
  {
    assert(v.first.empty()); // array elements have no names
    // Get scintillator lenght
    double scinLength = v.second.get("length", -1.0);
    int splitOnZ = std::round(scinLength / fCrystalSizeZ);
    int startSplit = -(splitOnZ / 2);
    int endSplit = (splitOnZ / 2);

    // Get position and rotation of the scintillator
    double pos_x = v.second.get("xcenter", -1.0);
    double pos_y = v.second.get("ycenter", -1.0);
    double pos_z = v.second.get("zcenter", -1.0);
    double rot_x = v.second.get("rot_x", -1.0);
    double rot_y = v.second.get("rot_y", -1.0);
    double rot_z = v.second.get("rot_z", -1.0);

    // std::cout << v.second.get("id", -1) << " " << v.second.get("xcenter", -1.0) << " " << v.second.get("ycenter", -1.0) << " "
    //           << v.second.get("rot_x", -1.0) << " " << v.second.get("rot_y", -1.0) << " " << v.second.get("rot_z", -1.0) << " " << std::endl;

    // Divide the strip into smaller cristals
    // CASToR uses milimeters as default units and Framework centimeter, converting [cm]->[mm]
    for (int j = startSplit; j < endSplit; j++)
    {
      output.push_back(10.0 * pos_x);
      output.push_back(10.0 * pos_y);
      output.push_back(10.0 * (fCrystalSizeZ * j));
      output.push_back(rot_x);
      output.push_back(rot_y);
      output.push_back(rot_z);
    }
  }

  // Rewrite the vector to the array
  int arraySize = output.size();
  float* outputArray = new float[arraySize];
  unsigned int i = 0;
  for (unsigned int i = 0; i < output.size(); ++i)
  {
    outputArray[i] = output.at(i);
  }

  // Save the resulting array
  FILE* LUT_file = fopen(lutFileName.c_str(), "wb");
  if (LUT_file == NULL)
  {
    std::cerr << "Failed to open " << lutFileName << " file" << std::endl;
  }

  if (fwrite(outputArray, sizeof(float), arraySize, LUT_file) != arraySize)
  {
    std::cerr << "Failed to write correct number of elements to file!" << std::endl;
  }
  else
  {
    std::cout << "LUT file saved. Number of created crystals: " << output.size() / 6 << std::endl;
  }
}
