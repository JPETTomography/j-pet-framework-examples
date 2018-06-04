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

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE(backProjectSinogramNone)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramNone.ppm";
  /// read phantom
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  std::vector<std::vector<double>> sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  JPetFilterNone noneFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, noneFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramRamLak)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramRamLak.ppm";
  /// read phantom
  std::ifstream in(inFile);
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
  JPetFilterRamLak ramLakFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, ramLakFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramCosine)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramCosine.ppm";
  /// read phantom
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  std::vector<std::vector<double>> sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  JPetFilterCosine cosineFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, cosineFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramHamming)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramHamming.ppm";
  /// read phantom
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  std::vector<std::vector<double>> sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  JPetFilterHamming hammingFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, hammingFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramRidgelet)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramRidgelet.ppm";
  /// read phantom
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  std::vector<std::vector<double>> sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  JPetFilterRidgelet ridgeletFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, ridgeletFilter, sinogram);
  int maxValueF = 0;
  for (unsigned int i = 0; i < filteredSinogram.size(); i++) {
    for (unsigned int j = 0; j < filteredSinogram[0].size(); j++) {
      if (static_cast<int>(filteredSinogram[i][j]) > maxValueF)
        maxValueF = static_cast<int>(filteredSinogram[i][j]);
    }
  }
  std::ofstream resF("filteredSinogram.ppm");
  resF << "P2" << std::endl;
  resF << filteredSinogram[0].size() << " " << filteredSinogram.size() << std::endl;
  resF << maxValueF << std::endl;
  for (unsigned int i = 0; i < filteredSinogram.size(); i++) {
    for (unsigned int j = 0; j < filteredSinogram[0].size(); j++) {
      int resultInt = static_cast<int>(filteredSinogram[i][j]);
      if (resultInt < 0) {
        resultInt = 0;
      }
      resF << resultInt << " ";
    }
    resF << std::endl;
  }
  resF.close();
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_CASE(backProjectSinogramSheppLogan)
{
  const auto inFile = "unitTestData/JPetRecoImageToolsTest/sinogramBackproject.ppm";
  const auto outFile = "backprojectSinogramSheppLogan.ppm";
  /// read phantom
  std::ifstream in(inFile);
  BOOST_REQUIRE(in);
  std::string line;
  getline(in, line);
  unsigned int width;
  unsigned int height;
  in >> width;
  in >> height;
  int val;
  in >> val; // skip max val
  std::vector<std::vector<double>> sinogram(height, std::vector<double>(width));
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      in >> val;
      sinogram[i][j] = val;
    }
  }
  JPetFilterSheppLogan shepploganFilter;
  JPetRecoImageTools::FourierTransformFunction f = JPetRecoImageTools::doFFTW;
  JPetRecoImageTools::Matrix2DProj filteredSinogram =
    JPetRecoImageTools::FilterSinogram(f, shepploganFilter, sinogram);
  JPetRecoImageTools::Matrix2DProj result = JPetRecoImageTools::backProject(
        filteredSinogram, width, JPetRecoImageTools::nonRescale, 0, 255);
  int maxValue = 0;
  for (unsigned int i = 0; i < result.size(); i++) {
    for (unsigned int j = 0; j < result[0].size(); j++) {
      if (static_cast<int>(result[i][j]) > maxValue)
        maxValue = static_cast<int>(result[i][j]);
    }
  }
  std::ofstream res(outFile);
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
  BOOST_REQUIRE(JPetCommonTools::ifFileExisting(outFile));
}

BOOST_AUTO_TEST_SUITE_END()