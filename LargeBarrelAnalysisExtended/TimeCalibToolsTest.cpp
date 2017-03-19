#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimeCalibTools
#include <boost/test/unit_test.hpp>

#include "TimeCalibTools.h"


struct myFixtures {
  std::vector<TimeCalibRecord> fCorrectRecords = {
    {
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .threshold = 1,
      .offset_value_leading = 7 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .threshold = 4,
      .offset_value_leading = 5 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .threshold = 2,
      .offset_value_leading = -3 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    }
  };
  std::map<std::tuple<int, int, JPetPM::Side, int>, int> fCorrectTombMap = {{std::make_tuple(1, 1, JPetPM::SideA, 1), 22},
    {std::make_tuple(3, 2, JPetPM::SideB, 4), 13},
    {std::make_tuple(2, 90, JPetPM::SideB, 2), 73}
  };
};

BOOST_AUTO_TEST_SUITE (TimeCalibToolsSuite)

BOOST_AUTO_TEST_CASE (getTimeCalibCorrection)
{
  std::map<unsigned int, double> calibration = { {0, 0.5}, {1, -1.2}, {3, 0.2}, {4, -0.1}};
  auto epsilon = 0.00001;
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 0), 0.5, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 1), -1.2, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 3), 0.2, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 4), -0.1, epsilon);

  /// Nonexisting should return 0
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 2),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 10),  0.0, epsilon);
}

BOOST_AUTO_TEST_CASE (getTimeCalibCorrection2)
{
  std::map<unsigned int, double> calibration;
  auto epsilon = 0.00001;
  /// Nonexisting should return 0
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 0),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 1),  0.0, epsilon);
  BOOST_REQUIRE_CLOSE(TimeCalibTools::getTimeCalibCorrection(calibration , 2),  0.0, epsilon);
}


BOOST_AUTO_TEST_CASE (loadTimeCalibration)
{
  std::map<std::tuple<int, int, JPetPM::Side, int>, int> tombMap = {{std::make_tuple(1, 10, JPetPM::SideB, 1), 10},
    {std::make_tuple(1, 11, JPetPM::SideA, 2), 11},
    {std::make_tuple(1, 12, JPetPM::SideB, 3), 12},
    {std::make_tuple(1, 13, JPetPM::SideA, 4), 13}
  };
  std::map<unsigned int, double> expected = { {10, -6.9443}, {11, -6.96546}, {12, -6.68968}, {13, -6.69345}};
  auto obtained = TimeCalibTools::loadTimeCalibration("configFiles/timeCalib.txt", tombMap);
  BOOST_REQUIRE_EQUAL(expected.size(), obtained.size());
  BOOST_REQUIRE(expected == obtained);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_empty_string)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1};
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value_leading, result.offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_leading, result.offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_value_trailing, result.offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_trailing, result.offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.quality, result.quality, epsilon);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_ok_string)
{
  TimeCalibRecord expected = {1, 2, JPetPM::SideB, 4, 4.3, 12.4, 3.3, 4.4, 1.5};
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1 };
  BOOST_REQUIRE(TimeCalibTools::fillTimeCalibRecord("1 2 B 4 4.3 12.4 3.3 4.4 1.5", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value_leading, result.offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_leading, result.offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_value_trailing, result.offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_trailing, result.offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.quality, result.quality, epsilon);
}


BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_bad_side)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1};
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("1 2 C 4 4.3 12.4 3.3 4.4 1.5", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value_leading, result.offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_leading, result.offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_value_trailing, result.offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_trailing, result.offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.quality, result.quality, epsilon);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_too_few_numbers)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1};
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0, 0.0, -1.0, -1 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("1 2 B 4 4.3", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value_leading, result.offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_leading, result.offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_value_trailing, result.offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty_trailing, result.offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected.quality, result.quality, epsilon);
}

BOOST_AUTO_TEST_CASE (readCalibrationRecordsFromFile)
{
  auto result = TimeCalibTools::readCalibrationRecordsFromFile("configFiles/timeCalib.txt");
  BOOST_REQUIRE_EQUAL(result.size(), 4u);
  auto epsilon = 0.00001;
  std::vector<TimeCalibRecord> expected = {
    { 1, 10, JPetPM::SideB, 1, -6.9443, 0.0318206, 0, 0, 0},
    { 1, 11, JPetPM::SideA, 2, -6.96546, 0.0306839, 0, 0, 0 },
    { 1, 12, JPetPM::SideB, 3, -6.68968, 0.0283943, 0, 0, 0},
    { 1, 13, JPetPM::SideA, 4, -6.69345, 0.0303125, 0, 0, 0}
  };

  BOOST_REQUIRE_EQUAL(expected[0].layer, result[0].layer);
  BOOST_REQUIRE_EQUAL(expected[0].slot, result[0].slot);
  BOOST_REQUIRE_EQUAL(expected[0].side, result[0].side);
  BOOST_REQUIRE_EQUAL(expected[0].threshold, result[0].threshold);
  BOOST_REQUIRE_CLOSE(expected[0].offset_value_leading, result[0].offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[0].offset_uncertainty_leading, result[0].offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[0].offset_value_trailing, result[0].offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[0].offset_uncertainty_trailing, result[0].offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[0].quality, result[0].quality, epsilon);

  BOOST_REQUIRE_EQUAL(expected[1].layer, result[1].layer);
  BOOST_REQUIRE_EQUAL(expected[1].slot, result[1].slot);
  BOOST_REQUIRE_EQUAL(expected[1].side, result[1].side);
  BOOST_REQUIRE_EQUAL(expected[1].threshold, result[1].threshold);
  BOOST_REQUIRE_CLOSE(expected[1].offset_value_leading, result[1].offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[1].offset_uncertainty_leading, result[1].offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[1].offset_value_trailing, result[1].offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[1].offset_uncertainty_trailing, result[1].offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[1].quality, result[1].quality, epsilon);

  BOOST_REQUIRE_EQUAL(expected[2].layer, result[2].layer);
  BOOST_REQUIRE_EQUAL(expected[2].slot, result[2].slot);
  BOOST_REQUIRE_EQUAL(expected[2].side, result[2].side);
  BOOST_REQUIRE_EQUAL(expected[2].threshold, result[2].threshold);
  BOOST_REQUIRE_CLOSE(expected[2].offset_value_leading, result[2].offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[2].offset_uncertainty_leading, result[2].offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[2].offset_value_trailing, result[2].offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[2].offset_uncertainty_trailing, result[2].offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[2].quality, result[2].quality, epsilon);

  BOOST_REQUIRE_EQUAL(expected[3].layer, result[3].layer);
  BOOST_REQUIRE_EQUAL(expected[3].slot, result[3].slot);
  BOOST_REQUIRE_EQUAL(expected[3].side, result[3].side);
  BOOST_REQUIRE_EQUAL(expected[3].threshold, result[3].threshold);
  BOOST_REQUIRE_CLOSE(expected[3].offset_value_leading, result[3].offset_value_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[3].offset_uncertainty_leading, result[3].offset_uncertainty_leading, epsilon);
  BOOST_REQUIRE_CLOSE(expected[3].offset_value_trailing, result[3].offset_value_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[3].offset_uncertainty_trailing, result[3].offset_uncertainty_trailing, epsilon);
  BOOST_REQUIRE_CLOSE(expected[3].quality, result[3].quality, epsilon);
}

BOOST_AUTO_TEST_CASE (readCalibrationRecordsFromFile_no_file)
{
  auto obtained = TimeCalibTools::readCalibrationRecordsFromFile("blabal.txt");
  BOOST_REQUIRE(obtained.empty());
}

BOOST_AUTO_TEST_CASE (areCorrectTimeCalibRecords)
{
  std::vector<TimeCalibRecord> records = {
    {
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .threshold = 1,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .threshold = 4,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .threshold = 2,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    }
  };
  BOOST_REQUIRE(TimeCalibTools::areCorrectTimeCalibRecords(records));
}

BOOST_AUTO_TEST_CASE (areCorrectTimeCalibRecords_wrong)
{
  std::vector<TimeCalibRecord> records = {
    {
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .threshold = 1,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .threshold = 4,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 2,
      .slot = 90,
      .side = JPetPM::SideB,
      .threshold = 5,   /// threshold >4
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    }
  };
  BOOST_REQUIRE(!TimeCalibTools::areCorrectTimeCalibRecords(records));
}

BOOST_AUTO_TEST_CASE (areCorrectTimeCalibRecords_wrong2)
{
  std::vector<TimeCalibRecord> records = {
    {
      .layer = 1,
      .slot = 1,
      .side = JPetPM::SideA,
      .threshold = 1,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 3,
      .slot = 2,
      .side = JPetPM::SideB,
      .threshold = 4,
      .offset_value_leading = 0 ,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    },
    {
      .layer = 4, /// layer > 3
      .slot = 90,
      .side = JPetPM::SideB,
      .threshold = 4,
      .offset_value_leading = 0,
      .offset_uncertainty_leading = 0,
      .offset_value_trailing = 0,
      .offset_uncertainty_trailing = 0,
      .quality = 0
    }
  };
  BOOST_REQUIRE(!TimeCalibTools::areCorrectTimeCalibRecords(records));
}

BOOST_AUTO_TEST_CASE (generateTimeCalibration_empty)
{
  std::vector<TimeCalibRecord> records;
  std::map<std::tuple<int, int, JPetPM::Side, int>, int> tombMap;
  auto calibration = TimeCalibTools::generateTimeCalibration(records, tombMap);
  BOOST_REQUIRE(calibration.empty());
}

BOOST_FIXTURE_TEST_CASE (generateTimeCalibration, myFixtures )
{
  auto epsilon = 0.0001;
  auto calibration = TimeCalibTools::generateTimeCalibration(fCorrectRecords, fCorrectTombMap);
  BOOST_REQUIRE_EQUAL(calibration.size(), 3u);
  BOOST_REQUIRE_EQUAL(calibration.count(22), 1);
  BOOST_REQUIRE_EQUAL(calibration.count(13), 1);
  BOOST_REQUIRE_EQUAL(calibration.count(73), 1);
  BOOST_REQUIRE_CLOSE(calibration.at(22), 7, epsilon);
  BOOST_REQUIRE_CLOSE(calibration.at(13), 5, epsilon);
  BOOST_REQUIRE_CLOSE(calibration.at(73), -3, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
