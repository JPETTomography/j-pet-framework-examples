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
  std::map<unsigned int, double> expected = { {0, 0.5}, {1, -1.2}, {3, 0.2}, {4, -0.1}}; 
  auto obtained = TimeCalibTools::loadTimeCalibration("calibTest.json");
  BOOST_REQUIRE_EQUAL(expected.size(), obtained.size());
  BOOST_REQUIRE(expected == obtained);
}

BOOST_AUTO_TEST_SUITE_END()
