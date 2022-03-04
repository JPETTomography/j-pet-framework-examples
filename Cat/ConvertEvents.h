#ifndef CONVERTEVENTS_H
#define CONVERTEVENTS_H

#include "JPetUserTask/JPetUserTask.h"
#include "JPetEvent/JPetEvent.h"
#include <fstream>
#include <iostream>

/**
 * This class is responsible for converting events
 * from J-PET framework format into CASToR format.
 * It requires already generated CASToR LUT file
 * describing used geometry. Additionally it reqires
 * information about number of crystals in LUT file and
 * crystals size.
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
  void setUpOptions();
  bool generateLUTFile(const std::string& outputPath);

  std::ofstream fOutputStream;
  std::vector<std::tuple<float, float, float>> fCastorIDs;
  uint32_t fNumberOfEvents   = 0;
  uint32_t fNumberOfCrystals = 0;
  float fCrystalSizeX = 0.f;
  float fCrystalSizeY = 0.f;
  float fCrystalSizeZ = 0.f;
  int fTOFFWHM             = 600;
  int fTOFMeasurementRange = 1500;
  std::string fInputFilePath  = "scanner";
  std::string fOutputFilePath = "scanner";
  std::string fScannerName    = "scanner";
  const std::string kNumberOfCrystalsKey    = "ConvertEvents_NumberOfCrystals_int";
  const std::string kNumberOfEventsKey      = "ConvertEvents_NumberOfEvents_int";
  const std::string kInputFilePathKey       = "ConvertEvents_LUTScannerPath_std::string";
  const std::string kScannerNameKey         = "ConvertEvents_ScannerName_std::string";
  const std::string kOutputFilePathKey      = "ConvertEvents_OutputConvertedPath_std::string";
  const std::string kCrystalSizeXKey        = "ConvertEvents_CrystalSizeX_mm_float";
  const std::string kCrystalSizeYKey        = "ConvertEvents_CrystalSizeY_mm_float";
  const std::string kCrystalSizeZKey        = "ConvertEvents_CrystalSizeZ_mm_float";
  const std::string kTOFFWHMpsKey           = "ConvertEvents_TOFFWHM_ps_int";
  const std::string kTOFMeasurementRangeKey = "ConvertEvents_TOFMeasurementRange_ps_int";
  std::string fOutFileName  = "";
  std::string fOutFilePath = "";
};
#endif
