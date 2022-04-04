#ifndef CONVERTEVENTSTOOLS_H
#define CONVERTEVENTSTOOLS_H

#include <string>
#include <vector>

class ConvertEventsTools
{
  public:
    static bool readLUTFile(const std::string& path, int numberOfCrystals, std::vector<std::tuple<float, float, float>> &readedLutGeometry);
    static uint32_t getCastorID(float x, float y, float z, const std::vector<std::tuple<float, float, float>> &castorIDs);
};

#endif
