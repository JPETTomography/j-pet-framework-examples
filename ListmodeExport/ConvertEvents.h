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
 *  @file ConvertEvents.h
 */

#ifndef CONVERTEVENTS_H
#define CONVERTEVENTS_H

#include "../ModularDetectorAnalysis/EventCategorizerTools.h"
#include "JPetEvent/JPetEvent.h"
#include "JPetUserTask/JPetUserTask.h"
#include <fstream>
#include <iostream>

/**
 * This class is responsible for converting events from J-PET framework format
 * into CASToR format.It requires already generated CASToR LUT file describing
 * used geometry. Additionally it reqires information about number of crystals
 * in LUT file and crystals size.
 *
 * As an input it takes uncategorized events (and it only
 * takes events that have exacly 2 hits).
 *
 * As an output it generates *.Cdf and .Cdh files
 * that can be used in CASToR for reconstruction.
 *
 * CASToR general documentation about LUT file and CASToR
 * format: https://castor-project.org/sites/default/files/2020-09/CASToR_general_documentation.pdf
 */
class ConvertEvents : public JPetUserTask
{

public:
  ConvertEvents(const char* name);
  virtual ~ConvertEvents();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  const std::string kNumberOfCrystalsKey = "ConvertEvents_NumberOfCrystals_int";

  const std::string kLUTFileNameKey = "ConvertEvents_LUTFileName_std::string";
  const std::string kCDHFileNameKey = "ConvertEvents_CDHFileName_std::string";
  const std::string kCDFFileNameKey = "ConvertEvents_CDFFileName_std::string";

  const std::string kCrystalSizeXKey = "ConvertEvents_CrystalSizeX_cm_double";
  const std::string kCrystalSizeYKey = "ConvertEvents_CrystalSizeY_cm_double";
  const std::string kCrystalSizeZKey = "ConvertEvents_CrystalSizeZ_cm_double";

  const std::string kScannerNameKey = "ConvertEvents_ScannerName_std::string";
  const std::string kTOFFWHMpsKey = "ConvertEvents_TOFFWHM_ps_int";
  const std::string kTOFMeasurementRangeKey = "ConvertEvents_TOFMeasurementRange_ps_int";

  void setUpOptions();

  std::ofstream fOutputStream;
  std::vector<TVector3> fCrystalsGeometry;

  // Converted LOR counter
  uint32_t fNumberOfLORs = 0;
  // The number of crystals included in LUT file - a crosscheck for files consistency
  uint32_t fNumberOfCrystals = 0;

  // Sizes of the 'crystals' - the volumes for CASToR reconstruction
  // J-PET scintillators are divided into the crystals in LUT configuration
  double fCrystalSizeX = 0.6f;
  double fCrystalSizeY = 2.5f;
  double fCrystalSizeZ = 5.0f;

  int fTOFFWHM = 600;
  int fTOFMeasurementRange = 1500;
  std::string fScannerName = "scanner";

  // CASToR files - LUT with the detector geometry (binary)
  // Cdh - ASCII header file with basic info about the detector
  // Cdf - binary data file with LORs in CASToR format
  std::string fLUTFileName = "jpetDector.lut";
  std::string fCdhFileName = "auto";
  std::string fCdfFileName = "auto";
};

#endif /* !CONVERTEVENTS_H */
