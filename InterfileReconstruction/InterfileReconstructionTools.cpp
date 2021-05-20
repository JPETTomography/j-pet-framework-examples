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
 */
#include "InterfileReconstructionTools.h"
#include "JPetLoggerInclude.h"
#include <fstream>

void InterfileReconstructionTools::saveToInterfile(TH3D* image, const std::string& outFileName) {
  if(image == nullptr) {
    ERROR("Missing pointer to image data!");
    return;
  }
  std::ofstream fOutputStream;
  fOutputStream.open(outFileName + ".img", std::ios::binary | std::ios::out);

  auto x_bins = image->GetNbinsX();
  auto y_bins = image->GetNbinsY();
  auto z_bins = image->GetNbinsZ();

  //Intefile format is defined in a way, that we saves 2D arrays
  //from top left corner to bottom right
  //in row major order, more information in article:
  //https://www.researchgate.net/publication/21707960_A_file_format_for_the_exchange_of_nuclear_medicine_image_data_A_specification_of_interfile_version_33

  for (auto i = 0; i < z_bins; i++) {
    for (auto j = y_bins; j > 0; j--) {
      auto bin_pos = image->GetBin(1, j, i + 1);
      for (auto k = 0; k < x_bins; k++) {
        auto val = static_cast<float>(image->GetBinContent(bin_pos + k));
        fOutputStream.write(reinterpret_cast<char *>(&val), sizeof(val));
      }
    }
  }

  fOutputStream.close();

  fOutputStream.open(outFileName + ".hdr", std::ios::out);
  fOutputStream << "X size: " << x_bins << " " << std::endl
                << "Y size: " << y_bins << " " << std::endl
                << "Z size: " << z_bins << " " << std::endl
                << "type: float" << std::endl;
  fOutputStream.close();
}
