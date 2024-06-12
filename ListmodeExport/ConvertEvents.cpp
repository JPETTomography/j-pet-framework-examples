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
 *  @file ConvertEvents.cpp
 */

#include <boost/property_tree/json_parser.hpp>

#include "ConvertEvents.h"
#include "ConvertEventsTools.h"
#include <iostream>
#include <limits>
#include <map>

using namespace jpet_options_tools;

ConvertEvents::ConvertEvents(const char* name) : JPetUserTask(name) {}

ConvertEvents::~ConvertEvents() {}

bool ConvertEvents::init()
{
  /// Reading user options
  setUpOptions();

  /// this flag is used to either create/override output file for first of the input files,
  /// or to append to the existing output file for other input files.
  static std::ios_base::openmode fOpenFlags = std::ios::binary | std::ios::out;

  /// Output ROOT file
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  /// Output CDF CASToR file
  fOutputStream.open(fCdfFileName, fOpenFlags);
  /// for processing other then first file, append data to existing output file
  if (fOpenFlags == std::ios::binary | std::ios::out)
  {
    fOpenFlags = std::ios::binary | std::ios::out | std::ios::app;
  }

  /// Reading LUT file to translate it to geometry of volumes ('crystals')
  fCrystalsGeometry = ConvertEventsTools::readLUTFile(fLUTFileName, fNumberOfCrystals);

  return true;
}

bool ConvertEvents::exec()
{
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++)
    {
      const auto lor = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));

      // Processing only events that were categorized as 2 gamma and have exacly 2 hits
      if (!(lor.isTypeOf(JPetEventType::k2Gamma) && lor.getHits().size() == 2))
      {
        continue;
      }

      auto firstHit = dynamic_cast<const JPetPhysRecoHit*>(lor.getHits().at(0));
      auto secondHit = dynamic_cast<const JPetPhysRecoHit*>(lor.getHits().at(1));
      if (!firstHit || !secondHit)
      {
        continue;
      }

      auto time = (firstHit->getTime() + secondHit->getTime()) / 2.0;
      auto tof = EventCategorizerTools::calculateTOFByConvention(firstHit, secondHit);
      uint32_t crystalID1 = ConvertEventsTools::getCrystalID(firstHit->getPos(), fCrystalsGeometry, fCrystalSizeZ);
      uint32_t crystalID2 = ConvertEventsTools::getCrystalID(secondHit->getPos(), fCrystalsGeometry, fCrystalSizeZ);

      /// In case of error - skip this LOR
      if (crystalID1 == -1 || crystalID2 == -1)
      {
        WARNING("LOR conversion error - crystal for hit not found");
        continue;
      }

      /// Appending to the output file
      fOutputStream.write(reinterpret_cast<char*>(&time), sizeof(time));
      fOutputStream.write(reinterpret_cast<char*>(&tof), sizeof(tof));
      fOutputStream.write(reinterpret_cast<char*>(&crystalID1), sizeof(crystalID1));
      fOutputStream.write(reinterpret_cast<char*>(&crystalID2), sizeof(crystalID2));
      fNumberOfLORs++;
    }
  }
  else
  {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool ConvertEvents::terminate()
{
  /// Closing CDF Castor file
  fOutputStream.close();

  INFO(Form("Number of LOR converted to list mode: %d", fNumberOfLORs));

  /// Overwriting the header file with the udated information
  fOutputStream.open(fCdhFileName, std::ios::out);
  fOutputStream << "Scanner name: " << fScannerName << std::endl
                << "Data filename: " << fCdfFileName << std::endl
                << "Number of events: " << fNumberOfLORs << std::endl
                << "Data mode: list-mode" << std::endl
                << "Data type: PET" << std::endl
                << "Start time (s): 0" << std::endl // dummy values of start and finish time
                << "Duration (s): 10" << std::endl  // TODO: Can be obtained from first and last processed file names
                << "TOF information flag: 1" << std::endl
                << "TOF resolution (ps): " << fTOFFWHM << std::endl
                << "List TOF measurement range (ps): " << fTOFMeasurementRange << std::endl;
  fOutputStream.close();
  return true;
}

void ConvertEvents::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kNumberOfCrystalsKey))
  {
    fNumberOfCrystals = getOptionAsInt(opts, kNumberOfCrystalsKey);
  }

  if (isOptionSet(opts, kLUTFileNameKey))
  {
    fLUTFileName = getOptionAsString(opts, kLUTFileNameKey);
  }

  if (isOptionSet(opts, kScannerNameKey))
  {
    fScannerName = getOptionAsString(opts, kScannerNameKey);
  }

  if (isOptionSet(opts, kCrystalSizeXKey))
  {
    fCrystalSizeX = getOptionAsDouble(opts, kCrystalSizeXKey);
  }
  if (isOptionSet(opts, kCrystalSizeYKey))
  {
    fCrystalSizeY = getOptionAsDouble(opts, kCrystalSizeYKey);
  }
  if (isOptionSet(opts, kCrystalSizeZKey))
  {
    fCrystalSizeZ = getOptionAsDouble(opts, kCrystalSizeZKey);
  }

  if (isOptionSet(opts, kTOFFWHMpsKey))
  {
    fTOFFWHM = getOptionAsInt(opts, kTOFFWHMpsKey);
  }
  if (isOptionSet(opts, kTOFMeasurementRangeKey))
  {
    fTOFMeasurementRange = getOptionAsInt(opts, kTOFMeasurementRangeKey);
  }

  // Setting the names of output CDH and CDF files
  // If the user setting is "auto" - the filne name will have the same name as
  // the input ROOT files with a .cdf and .cdh extentions
  // In the other case will be named as put in the params by the user
  if (isOptionSet(opts, kCDHFileNameKey))
  {
    fCdhFileName = getOptionAsString(opts, kCDHFileNameKey);
  }
  if (fCdhFileName == "auto")
  {
    auto inputFileName = getOptionAsString(fParams.getOptions(), "inputFile_std::string");
    inputFileName.replace(inputFileName.find(".cat.evt.root"), std::string::npos, ".Cdh");
    fCdhFileName = inputFileName;
  }

  if (isOptionSet(opts, kCDFFileNameKey))
  {
    fCdfFileName = getOptionAsString(opts, kCDFFileNameKey);
  }
  if (fCdfFileName == "auto")
  {
    auto inputFileName = getOptionAsString(fParams.getOptions(), "inputFile_std::string");
    inputFileName.replace(inputFileName.find(".cat.evt.root"), std::string::npos, ".Cdf");
    fCdfFileName = inputFileName;
  }
}
