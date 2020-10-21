#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE JPetRecoImageToolsTests
#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include "./JPetFilterCosine.h"
#include "./JPetFilterHamming.h"
#include "./JPetFilterNone.h"
#include "./JPetFilterRamLak.h"
#include "./JPetFilterRidgelet.h"
#include "./JPetFilterSheppLogan.h"
#include "./JPetRecoImageTools.h"
#include "JPetCommonTools/JPetCommonTools.h"

const std::string test_files_path = "unitTestData/JPetRecoImageToolsTest/";

const auto inFile = std::vector<std::string>{"rsinogram_slice_0_0.000000_to_0.833332.ppm",    "rsinogram_slice_1_0.833332_to_1.666666.ppm",
                                             "rsinogram_slice_2_1.666666_to_2.500000.ppm",    "rsinogram_slice_21_17.500000_to_18.333332.ppm",
                                             "rsinogram_slice_22_18.333332_to_19.166664.ppm", "rsinogram_slice_23_19.166664_to_20.000000.ppm",
                                             "rsinogram_slice_-1_-0.833334_to_0.000000.ppm",  "rsinogram_slice_-2_-1.666668_to_-0.833334.ppm"};

int getMaxValue(const JPetRecoImageTools::SparseMatrix& result)
{
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      if (static_cast<int>(result(i, j)) > maxValue)
        maxValue = static_cast<int>(result(i, j));
    }
  }
  return maxValue;
}

void saveResult(const JPetRecoImageTools::SparseMatrix& result, const std::string& outputFileName)
{
  int maxValue = getMaxValue(result);
  std::ofstream res(outputFileName);
  res << "P2" << std::endl;
  res << result.size2() << " " << result.size1() << std::endl;
  res << maxValue << std::endl;
  for (unsigned int i = 0; i < result.size1(); i++)
  {
    for (unsigned int j = 0; j < result.size2(); j++)
    {
      int resultInt = static_cast<int>(result(i, j));
      if (resultInt < 0)
      {
        resultInt = 0;
      }
      res << resultInt << " ";
    }
    res << std::endl;
  }
  res.close();
}

JPetRecoImageTools::SparseMatrix readFile(const std::string& inputFile)
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
  JPetRecoImageTools::SparseMatrix sinogram(height, width);
  for (unsigned int i = 0; i < height; i++)
  {
    for (unsigned int j = 0; j < width; j++)
    {
      in >> val;
      sinogram(i, j) = val;
    }
  }
  return sinogram;
}

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(backProjectSinogramRamLak)
{
  for (const auto& in : inFile)
  {
    for (double threshold = 0.01; threshold <= 1.; threshold += 0.01)
    {
      const auto outFile = "ramlak/" + in + "RamLakT" + std::to_string(threshold) + ".ppm";
      std::cout << "Reading file: " << test_files_path + in << " output file: " << outFile << std::endl;
      JPetRecoImageTools::SparseMatrix sinogram = readFile(test_files_path + in);

      JPetFilterRamLak ramLakFilter(threshold);
      JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
      JPetRecoImageTools::SparseMatrix filteredSinogram = JPetRecoImageTools::FilterSinogram(f, ramLakFilter, sinogram);
      JPetRecoImageTools::SparseMatrix result =
          JPetRecoImageTools::backProject(filteredSinogram, sinogram.size2(), JPetRecoImageTools::nonRescale, 0, 255);

      saveResult(result, outFile);

      BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()