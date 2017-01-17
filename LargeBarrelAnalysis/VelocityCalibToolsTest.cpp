#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE VelocityCalibTools
#include <boost/test/unit_test.hpp>

#include "VelocityCalibTools.h"
#include <JPetHit/JPetHit.h>

BOOST_AUTO_TEST_SUITE (VelocityCalibToolsSuite)

BOOST_AUTO_TEST_CASE (getVelocityWithEmptyCalib)
{
   std::map<unsigned int, double> calib;
   auto eps = 0.000001;
      
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 0), 0.0 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 3), 0.0 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 50), 0.0 ,eps);
}

BOOST_AUTO_TEST_CASE (getVelocityWithGivenCalib)
{
   std::map<unsigned int, double> calib = { {1, 14.0}, {2,-14.5}, {0, 0.1}, {5,-0.5} };
   auto eps = 0.000001;
   
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 1), 14.0 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 2), -14.5 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 0), 0.1 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 5), -0.5 ,eps);
   
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 3), 0.0 ,eps);
   BOOST_REQUIRE_CLOSE(VelocityCalibTools::getVelocity(calib, 50), 0.0 ,eps);
}

BOOST_AUTO_TEST_CASE (loadVelocityCalibration)
{
  std::map<unsigned int, double> expected = { {1, 14.0}, {2,-14.5}, {0, 0.1}, {5,-0.5} }; 
  auto obtained = VelocityCalibTools::loadVelocities("velocityTest.txt");
  BOOST_REQUIRE_EQUAL(expected.size(), obtained.size());
  BOOST_REQUIRE(expected == obtained);
}

BOOST_AUTO_TEST_CASE (applyVelocityCalib)
{
   JPetHit dummyHit;
   dummyHit.setTimeDiff(0);
   dummyHit.setScinID(1);
   
   std::map<unsigned int, double> calib = { {1, 14.0}, {2,-14.5}, {0, 0.1}, {5,-0.5} };
   
   auto eps = 0.000001;
   
   BOOST_REQUIRE_CLOSE( dummyHit.getTimeDiff() * VelocityCalibTools::getVelocity(calib, dummyHit.getScinID()) / 2.0 , 0.0, eps);
   
   
   dummyHit.setTimeDiff(5);
   BOOST_REQUIRE_CLOSE(dummyHit.getTimeDiff() * VelocityCalibTools::getVelocity(calib, dummyHit.getScinID()) / 2.0 , 35.0, eps );
   
   dummyHit.setTimeDiff(-6);
   BOOST_REQUIRE_CLOSE( dummyHit.getTimeDiff() * VelocityCalibTools::getVelocity(calib, dummyHit.getScinID()) / 2.0 , -42.0, eps );
}

BOOST_AUTO_TEST_SUITE_END()