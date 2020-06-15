#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE JPetRecoImageToolsTests
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include "JPetCommonTools/JPetCommonTools.h"
#include "./JPetFilterCosine.h"
#include "./JPetFilterHamming.h"
#include "./JPetFilterNone.h"
#include "./JPetFilterRamLak.h"
#include "./JPetFilterRidgelet.h"
#include "./JPetFilterSheppLogan.h"
#include "./JPetRecoImageTools.h"

const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";

int getMaxValue(const JPetRecoImageTools::Matrix2DProj& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  return maxValue;
}

void saveResult(const JPetRecoImageTools::Matrix2DProj& result, const std::string& outputFileName)
{
  int maxValue = getMaxValue(result);
  std::ofstream res(outputFileName);
  res << "P2" << std::endl;
  res << result[0].size() << " " << result.size() << std::endl;
  res << maxValue << std::endl;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      int resultInt = static_cast<int>(result[i][j]);
      if (resultInt < 0) {
        resultInt = 0;
      }
      res << resultInt << " ";
    }
    res << std::endl;
  }
  res.close();
}

JPetRecoImageTools::Matrix2DProj readFile(const std::string& inputFile)
{
  std::ifstream in(inputFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  JPetRecoImageTools::Matrix2DProj sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  return sinogram;
}

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(backProjectSinogramNone)
{
  const auto outFile = "backprojectSinogramNone.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterNone noneFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, noneFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramRamLak)
{
  const auto outFile = "backprojectSinogramRamLak.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterRamLak ramLakFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, ramLakFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramCosine)
{
  const auto outFile = "backprojectSinogramCosine.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterCosine cosineFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, cosineFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramHamming)
{
  const auto outFile = "backprojectSinogramHamming.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterHamming hammingFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, hammingFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramRidgelet)
{
  const auto outFile = "backprojectSinogramRidgelet.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterRidgelet ridgeletFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, ridgeletFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramSheppLogan)
{
  const auto outFile = "backprojectSinogramSheppLogan.ppm";

  JPetRecoImageTools::Matrix2DProj sinogram = readFile(inFile);

  JPetFilterSheppLogan shepploganFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, shepploganFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, sinogram[0].size(), JPetRecoImageTools::nonRescale, 0, 255);

  saveResult(result, outFile);

  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_SUITE_END()