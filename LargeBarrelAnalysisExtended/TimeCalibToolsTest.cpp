#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TimeCalibTools
#include <boost/test/unit_test.hpp>

#include "TimeCalibTools.h"

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
  std::map<unsigned int, double> expected = { {1, 1.1}, {2, 15}, {3, 1.2}, {5, -1.3}};
  auto obtained = TimeCalibTools::loadTimeCalibration("timeCalib.txt");
  BOOST_REQUIRE_EQUAL(expected.size(), obtained.size());
  BOOST_REQUIRE(expected == obtained);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_empty_string)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value, result.offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty, result.offset_uncertainty, epsilon);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_ok_string)
{
  TimeCalibRecord expected = {1, 2, JPetPM::SideB, 4, 4.3, 12.4};
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  BOOST_REQUIRE(TimeCalibTools::fillTimeCalibRecord("1 2 B 4 4.3 12.4", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value, result.offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty, result.offset_uncertainty, epsilon);
}


BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_bad_side)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("1 2 C 4 4.3 12.4", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value, result.offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty, result.offset_uncertainty, epsilon);
}

BOOST_AUTO_TEST_CASE (fillTimeCalibRecord_too_few_numbers)
{
  TimeCalibRecord expected = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  TimeCalibRecord result = { -1, -1, JPetPM::SideA, -1, 0.0, -1.0 };
  BOOST_REQUIRE(!TimeCalibTools::fillTimeCalibRecord("1 2 B 4 4.3", result));
  auto epsilon = 0.00001;
  BOOST_REQUIRE_EQUAL(expected.layer, result.layer);
  BOOST_REQUIRE_EQUAL(expected.slot, result.slot);
  BOOST_REQUIRE_EQUAL(expected.side, result.side);
  BOOST_REQUIRE_EQUAL(expected.threshold, result.threshold);
  BOOST_REQUIRE_CLOSE(expected.offset_value, result.offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected.offset_uncertainty, result.offset_uncertainty, epsilon);
}

BOOST_AUTO_TEST_CASE (readCalibrationRecordsFromFile)
{
  auto result = TimeCalibTools::readCalibrationRecordsFromFile("timeCalib.txt");
  BOOST_REQUIRE_EQUAL(result.size(), 4u);
  auto epsilon = 0.00001;
  std::vector<TimeCalibRecord> expected = {
    { 1, 1, JPetPM::SideA, 1, 1.1, 0.0 },
    { 1, 2, JPetPM::SideB, 1, 15, 0.5 },
    { 1, 3, JPetPM::SideA, 1, 1.2, 0.4 },
    { 1, 5, JPetPM::SideB, 1, -1.3, 0.2 }
  };

  BOOST_REQUIRE_EQUAL(expected[0].layer, result[0].layer);
  BOOST_REQUIRE_EQUAL(expected[0].slot, result[0].slot);
  BOOST_REQUIRE_EQUAL(expected[0].side, result[0].side);
  BOOST_REQUIRE_EQUAL(expected[0].threshold, result[0].threshold);
  BOOST_REQUIRE_CLOSE(expected[0].offset_value, result[0].offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected[0].offset_uncertainty, result[0].offset_uncertainty, epsilon);

  BOOST_REQUIRE_EQUAL(expected[1].layer, result[1].layer);
  BOOST_REQUIRE_EQUAL(expected[1].slot, result[1].slot);
  BOOST_REQUIRE_EQUAL(expected[1].side, result[1].side);
  BOOST_REQUIRE_EQUAL(expected[1].threshold, result[1].threshold);
  BOOST_REQUIRE_CLOSE(expected[1].offset_value, result[1].offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected[1].offset_uncertainty, result[1].offset_uncertainty, epsilon);

  BOOST_REQUIRE_EQUAL(expected[2].layer, result[2].layer);
  BOOST_REQUIRE_EQUAL(expected[2].slot, result[2].slot);
  BOOST_REQUIRE_EQUAL(expected[2].side, result[2].side);
  BOOST_REQUIRE_EQUAL(expected[2].threshold, result[2].threshold);
  BOOST_REQUIRE_CLOSE(expected[2].offset_value, result[2].offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected[2].offset_uncertainty, result[2].offset_uncertainty, epsilon);

  BOOST_REQUIRE_EQUAL(expected[3].layer, result[3].layer);
  BOOST_REQUIRE_EQUAL(expected[3].slot, result[3].slot);
  BOOST_REQUIRE_EQUAL(expected[3].side, result[3].side);
  BOOST_REQUIRE_EQUAL(expected[3].threshold, result[3].threshold);
  BOOST_REQUIRE_CLOSE(expected[3].offset_value, result[3].offset_value, epsilon);
  BOOST_REQUIRE_CLOSE(expected[3].offset_uncertainty, result[3].offset_uncertainty, epsilon);
}

BOOST_AUTO_TEST_CASE (readCalibrationRecordsFromFile_no_file)
{
  auto obtained = TimeCalibTools::readCalibrationRecordsFromFile("blabal.txt");
  BOOST_REQUIRE(obtained.empty());
}

BOOST_AUTO_TEST_SUITE_END()
