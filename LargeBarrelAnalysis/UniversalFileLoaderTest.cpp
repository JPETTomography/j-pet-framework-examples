#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE UniversalFileLoader

#include <boost/test/unit_test.hpp>
#include "UniversalFileLoader.h"

struct myFixtures {
  std::vector<ConfRecord> fCorrectRecords = {
    ConfRecord{
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .thresholdNumber = 1,
      .parameters = std::vector<double>{7.0,0.1,0.0,0.5,0.0,6.1,0.0,2.1}
    },
    ConfRecord{
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .thresholdNumber = 4,
      .parameters = std::vector<double>{5.0,0.3,0.0,0.3,0.0,9.1,0.0,3.1}
    },
    ConfRecord{
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .thresholdNumber = 2,
      .parameters = std::vector<double>{-3.0,0.2,4.0,0.3,0.0,2.1,0.0,4.5}
    }
  };
  std::map<std::tuple<int, int, JPetPM::Side, int>, int> fCorrectTombMap =
  {
    {std::make_tuple(1, 1, JPetPM::SideA, 1), 22},
    {std::make_tuple(3, 2, JPetPM::SideB, 4), 13},
    {std::make_tuple(2, 90, JPetPM::SideB, 2), 73}
  };
};

BOOST_AUTO_TEST_SUITE (UniversalFileLoaderSuite)

BOOST_AUTO_TEST_CASE (getConfigurationParameter)
{
  UniversalFileLoader::TOMBChToParameter configuration = {
    {0, std::vector<double>{0.5,0.1,0.0,0.5,0.0,6.1,0.0,2.1}},
    {1, std::vector<double>{-1.2,0.1,0.0,0.5,0.0,6.1,0.0,2.1}},
    {3, std::vector<double>{0.2,0.1,0.0,0.5,0.0,6.1,0.0,2.1}},
    {4, std::vector<double>{-0.1,0.1,0.0,0.5,0.0,6.1,0.0,2.1}}
  };
  auto epsilon = 0.00001;
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration, 0), 0.5, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration, 1), -1.2, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration, 3), 0.2, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration, 4), -0.1, epsilon);

  /// Nonexisting should return 0
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration , 2),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration , 10),  0.0, epsilon);
}

BOOST_AUTO_TEST_CASE (getConfigurationParameter2)
{
  UniversalFileLoader::TOMBChToParameter configuration;
  auto epsilon = 0.00001;
  /// Nonexisting should return 0
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration , 0),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration , 1),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(UniversalFileLoader::getConfigurationParameter(configuration , 2),  0.0, epsilon);
}

BOOST_AUTO_TEST_CASE (fillConfRecord_empty_string)
{
  ConfRecord expected = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  ConfRecord result = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  BOOST_REQUIRE(!UniversalFileLoader::fillConfRecord("", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.thresholdNumber, result.thresholdNumber);
  BOOST_REQUIRE_CLOSE(expected.parameters[0], result.parameters[0], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[1], result.parameters[1], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[2], result.parameters[2], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[3], result.parameters[3], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[4], result.parameters[4], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[5], result.parameters[5], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[6], result.parameters[6], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[7], result.parameters[7], epsilon);
}

BOOST_AUTO_TEST_CASE (fillConfRecord_ok_string)
{
  ConfRecord expected = {1, 2, JPetPM::SideB, 4, {0.0, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7}};
  ConfRecord result = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  BOOST_REQUIRE(UniversalFileLoader::fillConfRecord("1 2 B 4 0.0 1.1 2.2 3.3 4.4 5.5 6.6 7.7", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.thresholdNumber, result.thresholdNumber);
  BOOST_REQUIRE_CLOSE(expected.parameters[0], result.parameters[0], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[1], result.parameters[1], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[2], result.parameters[2], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[3], result.parameters[3], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[4], result.parameters[4], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[5], result.parameters[5], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[6], result.parameters[6], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[7], result.parameters[7], epsilon);
}


BOOST_AUTO_TEST_CASE (fillConfRecord_bad_side)
{
  ConfRecord expected = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  ConfRecord result = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  BOOST_REQUIRE(!UniversalFileLoader::fillConfRecord("1 2 C 4 0.0 1.1 2.2 3.3 4.4 5.5 6.6 7.7", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.thresholdNumber, result.thresholdNumber);
  BOOST_REQUIRE_CLOSE(expected.parameters[0], result.parameters[0], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[1], result.parameters[1], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[2], result.parameters[2], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[3], result.parameters[3], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[4], result.parameters[4], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[5], result.parameters[5], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[6], result.parameters[6], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[7], result.parameters[7], epsilon);
}

BOOST_AUTO_TEST_CASE (fillConfRecord_too_few_numbers)
{
  ConfRecord expected = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  ConfRecord result = {-1, -1, JPetPM::SideA, -1, {-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0,-1.0}};
  BOOST_REQUIRE(!UniversalFileLoader::fillConfRecord("1 2 B 4 4.3", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.thresholdNumber, result.thresholdNumber);
  BOOST_REQUIRE_CLOSE(expected.parameters[0], result.parameters[0], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[1], result.parameters[1], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[2], result.parameters[2], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[3], result.parameters[3], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[4], result.parameters[4], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[5], result.parameters[5], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[6], result.parameters[6], epsilon);
  BOOST_REQUIRE_CLOSE(expected.parameters[7], result.parameters[7], epsilon);
}

BOOST_AUTO_TEST_CASE (readConfigurationParametersFromFile)
{
  auto result = UniversalFileLoader::readConfigurationParametersFromFile("../dummyCalibration.txt");
  BOOST_REQUIRE_EQUAL(result.size(), 1536);
}

BOOST_AUTO_TEST_CASE (readConfigurationParametersFromFile_no_file)
{
  auto obtained = UniversalFileLoader::readConfigurationParametersFromFile("blabalbaahl.txt");
  BOOST_REQUIRE(obtained.empty());
}

BOOST_AUTO_TEST_CASE (areConfRecordsValid)
{
  std::vector<ConfRecord> records = {
    ConfRecord{
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .thresholdNumber = 1,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .thresholdNumber = 4,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .thresholdNumber = 2,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    }
  };
  BOOST_REQUIRE(UniversalFileLoader::areConfRecordsValid(records));
}

BOOST_AUTO_TEST_CASE (areConfRecordsValid_wrong)
{
  std::vector<ConfRecord> records = {
    ConfRecord{
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .thresholdNumber = 1,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .thresholdNumber = 4,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .thresholdNumber = 5, // threshold >4
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    }
  };
  BOOST_REQUIRE(!UniversalFileLoader::areConfRecordsValid(records));
}

BOOST_AUTO_TEST_CASE (areConfRecordsValid_wrong2)
{
  std::vector<ConfRecord> records = {
    ConfRecord{
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .thresholdNumber = 1,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .thresholdNumber = 4,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    },
    ConfRecord{
      .layer = 4, // layer > 3
      .slot = 90,
      .side = JPetPM::SideB,
      .thresholdNumber = 5,
      .parameters = std::vector<double>{0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    }
  };
  BOOST_REQUIRE(!UniversalFileLoader::areConfRecordsValid(records));
}

BOOST_AUTO_TEST_CASE(generateConfigurationParameters_empty)
{
  std::vector<ConfRecord> records;
  std::map<std::tuple<int, int, JPetPM::Side, int>, int> tombMap;
  auto configuration = UniversalFileLoader::generateConfigurationParameters(records, tombMap);
  BOOST_REQUIRE(configuration.empty());
}

BOOST_FIXTURE_TEST_CASE (generateConfigurationParameters, myFixtures)
{
  auto epsilon = 0.0001;
  auto configuration = UniversalFileLoader::generateConfigurationParameters(fCorrectRecords, fCorrectTombMap);
  BOOST_REQUIRE_EQUAL(configuration.size(), 3);
  BOOST_REQUIRE_EQUAL(configuration.count(22), 1);
  BOOST_REQUIRE_EQUAL(configuration.count(13), 1);
  BOOST_REQUIRE_EQUAL(configuration.count(73), 1);
  BOOST_REQUIRE_CLOSE(configuration.at(22).at(0), 7, epsilon);
  BOOST_REQUIRE_CLOSE(configuration.at(13).at(0), 5, epsilon);
  BOOST_REQUIRE_CLOSE(configuration.at(73).at(0), -3, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
