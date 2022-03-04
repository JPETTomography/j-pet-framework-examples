#include "ConvertEvents.h"
#include <map>
#include <limits>
#include "ConvertEventsTools.h"
using namespace jpet_options_tools;

ConvertEvents::ConvertEvents(const char* name) : JPetUserTask(name) {}

ConvertEvents::~ConvertEvents() {}

bool ConvertEvents::init()
{
  setUpOptions();

  static std::ios_base::openmode fOpenFlags = std::ios::binary | std::ios::out; // this flag is used to either create/override output file for first of the input files, or to append to the existing output file for other input files.
  // if we processing multiple files, open and override output file
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  fOutputStream.open(fOutputFilePath + ".Cdf", fOpenFlags);
  // for processing other then first file, app data to existing output file
  if (fOpenFlags == std::ios::binary | std::ios::out)
    fOpenFlags = std::ios::binary | std::ios::out | std::ios::app;
  std::string lutFile = fInputFilePath + ".lut";

  return ConvertEventsTools::readLUTFile(lutFile, fNumberOfCrystals, fCastorIDs);
}

bool ConvertEvents::exec()
{
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      const auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      const auto hits = event.getHits();
      if (hits.size() != 2)
      {
        continue;
      }
      const auto& firstHit = *hits[0];
      const auto& secondHit = *hits[1];

      // JPet Framework do not have information about time of hit acqusition, so just dummy value
      uint32_t time = 1;
      float tof = firstHit.getTime() - secondHit.getTime();
      // JPet Framework is storing position in cm, but castor uses mm, so multiply position by 10
      uint32_t castor1 = ConvertEventsTools::getCastorID(firstHit.getPosX()  * 10, firstHit.getPosY()  * 10, firstHit.getPosZ()  * 10, fCastorIDs);
      uint32_t castor2 = ConvertEventsTools::getCastorID(secondHit.getPosX() * 10, secondHit.getPosY() * 10, secondHit.getPosZ() * 10, fCastorIDs);

      // std::cout << "h1=[" << firstHit.getPos().X() << ", " << firstHit.getPos().Y() << ", " << firstHit.getPos().Z() << "] ";
      // std::cout << "id1=" << castor1 <<" ";
      // std::cout << "h2=[" << secondHit.getPos().X() << ", " << secondHit.getPos().Y() << ", " << secondHit.getPos().Z() << "] ";
      // std::cout << "id2=" << castor2;
      // std::cout << std::endl;
      
      fOutputStream.write(reinterpret_cast<char *>(&time), sizeof(time));
      fOutputStream.write(reinterpret_cast<char *>(&tof), sizeof(tof));
      fOutputStream.write(reinterpret_cast<char *>(&castor1), sizeof(castor1));
      fOutputStream.write(reinterpret_cast<char *>(&castor2), sizeof(castor2));
      fNumberOfEvents++;
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
  // static variable containing total number of events in all
  // single-run processed files
  static uint32_t totalEvents = 0;
  totalEvents += fNumberOfEvents;
  fOutputStream.close();
  std::string headerFile = fOutputFilePath + ".Cdh";
  fOutputStream.open(headerFile, std::ios::out);
  fOutputStream << "Scanner name: " << fScannerName << std::endl
       << "Data filename: " << fOutputFilePath << ".Cdf" << std::endl
       << "Number of events: " << totalEvents << std::endl
       << "Data mode: list-mode" << std::endl
       << "Data type: PET" << std::endl
       // here we are setting Start time/Duration to dummy values
       // JPet Framework do not have this kind of information
       // TODO: check if this value does not influence reconstruction
       // in some way
       << "Start time (s): 0" << std::endl
       << "Duration (s): 10" << std::endl
       << "TOF information flag: 1" << std::endl
       << "TOF resolution (ps): " << fTOFFWHM << std::endl
       << "List TOF measurement range (ps): " << fTOFMeasurementRange << std::endl;
  fOutputStream.close();
  return true;
}

/**
 * This function generates LUT file based on current ParamBank.
 * It maps all scintillators to binary file readed by CASTOR.
 *
 * Note: currently this function is not well-tested.
 *
 * @param outputPath path where LUT file should be saved.
 */
//Currently paramBank is returning invalid information about used geometry,
//so this function is wasn't tested yet. Disabling it for now.
/*bool ConvertEvents::generateLUTFile(const std::string& outputPath) {
  const auto paramBank = getParamBank();
  if (paramBank.isDummy()) {
    std::cout << "Returned param bank is dummy, aborting generating LUT file!" << std::endl;
    return false;
  }
  const auto scins = paramBank.getScins();
  std::map<int, std::vector<JPetScin>> scinMapped;
  // map scintillators by layer ID
  // assume lowest layer ID is first layer and then layer ID increases with physical layers
  for (auto scin : scins) {
    scinMapped[scin.second->getSlot().getLayer().getID()].push_back(*(scin.second));
  }

  //this assumes, that all scintillators have same length
  //TODO: add check that all scintillators have same length
  auto scinLength = scins.begin()->second[0].getLength();
  int splitOnZ = std::round(scinLength / fCrystalSizeZ);
  //C standard by default rounds towards 0, so for positive values it is floor,
  //for negatives it is ceil and we want to do that here.
  int startSplit = -(splitOnZ / 2);
  int endSplit = (splitOnZ / 2);
  //calculate size of required array to save all information about geometry
  //number of scintillators * how may times they are splitted on z * how may floats we need to save for each cristal (3 for position and 3 for rotation)
  //
  const int kInformationSize = 6;
  int arraySize = scinMapped.size() * splitOnZ * kInformationSize;
  float *outputArray = new float[arraySize];
  unsigned int i = 0;
  // map automatically sorts by key value
  for (auto scins : scinMapped) {
    for (auto scin : scins.second) {
      auto radius = scin.getSlot().getLayer().getRadius();
      auto theta = scin.getSlot().getTheta() * (M_PI / 180);
      for(int j = startSplit; j < endSplit; j++) {
        outputArray[i++] = radius * std::cos(theta);
        outputArray[i++] = radius * std::sin(theta);
        outputArray[i++] = fCrystalSizeZ * j;
        outputArray[i++] = theta;
        //This is wasn't tested. Probably we should also rotate here on ``y`` axis.
        //Please check this before enabling this function.
        outputArray[i++] = 0.;
        outputArray[i++] = 0.;
      }
    }
  }
  FILE* LUT_file = fopen(outputPath.c_str(), "wb");
  l (LUT_file==NULL)
l{
    std::cerr << "Failed to open " << outputPath << " file" << std::endl;
    return false;
  }
  if (fwrite(outputArray, sizeof(float), arraySize, LUT_file) != arraySize) {
    std::cerr << "Failed to write correct number of elements to file!" << std::endl;
    return false;
  }
  INFO("Used scintillator length: " << scinLength << " crystal size on z: " << fCrystalSizeZ << " number of crystals on Z: " << scinLength / fCrystalSizeZ << " (after round: " << splitOnZ << ") " << "starting split position: " << startSplit << " ending split position: " << endSplit << std::endl);
  return true;
}*/

void ConvertEvents::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kNumberOfCrystalsKey))
  {
    fNumberOfCrystals = getOptionAsInt(opts, kNumberOfCrystalsKey);
  }
  if (isOptionSet(opts, kNumberOfEventsKey))
  {
    fNumberOfEvents = getOptionAsInt(opts, kNumberOfEventsKey);
  }
  if (isOptionSet(opts, kInputFilePathKey))
  {
    fInputFilePath = getOptionAsString(opts, kInputFilePathKey);
  }
  if (isOptionSet(opts, kScannerNameKey))
  {
    fScannerName = getOptionAsString(opts, kScannerNameKey);
  }
  // if (isOptionSet(opts, kOutputFilePathKey))
  // {
  //   fOutputFilePath = getOptionAsString(opts, kOutputFilePathKey);
  // }
  if (isOptionSet(opts, kCrystalSizeXKey))
  {
    fCrystalSizeX = getOptionAsFloat(opts, kCrystalSizeXKey);
  }
  if (isOptionSet(opts, kCrystalSizeYKey))
  {
    fCrystalSizeY = getOptionAsFloat(opts, kCrystalSizeYKey);
  }
  if (isOptionSet(opts, kCrystalSizeZKey))
  {
    fCrystalSizeZ = getOptionAsFloat(opts, kCrystalSizeZKey);
  }
  if (isOptionSet(opts, kTOFFWHMpsKey))
  {
    fTOFFWHM = getOptionAsInt(opts, kTOFFWHMpsKey);
  }
  if (isOptionSet(opts, kTOFMeasurementRangeKey))
  {
    fTOFMeasurementRange = getOptionAsInt(opts, kTOFMeasurementRangeKey);
  }

  // handling unique output file names
  if(isOptionSet(fParams.getOptions(), "inputFile_std::string")){
    fOutFileName = getOptionAsString(fParams.getOptions(), "inputFile_std::string");    
  }
  
  if(isOptionSet(fParams.getOptions(), "outputPath_std::string")){
    fOutFilePath = getOptionAsString(fParams.getOptions(), "outputPath_std::string");    
  }
  
  // initialize output file and tree
  if(fOutFileName.find(".root") != std::string::npos){
    fOutFileName.replace(fOutFileName.find(".root"), std::string::npos, "");    
  }
  fOutputFilePath = fOutFileName;

  if (!fOutFilePath.empty()) {
    size_t filename_pos = fOutFileName.find("dabc");
    fOutFileName.replace(0, filename_pos - 1, fOutFilePath);
  }

}
