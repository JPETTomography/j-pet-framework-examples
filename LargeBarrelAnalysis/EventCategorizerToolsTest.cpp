#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EventCategorizerToolsTests
#include <boost/test/unit_test.hpp>

#include "EventCategorizerTools.h"
#include "JPetHit/JPetHit.h"

BOOST_AUTO_TEST_SUITE(TOFSuite)

BOOST_AUTO_TEST_CASE(checkHitOrder)
{
  JPetHit firstHit;
  firstHit.setTime(500.0);

  JPetHit secondHit;
  secondHit.setTime(100);
  
  double tof = EventCategorizerTools::calculateTOF(firstHit, secondHit);
  
  BOOST_REQUIRE_CLOSE( tof, 9999, 0.100);
}

BOOST_AUTO_TEST_CASE(checkTOFsignNegative)
{
  JPetHit firstHit;
  firstHit.setTime(100);
  JPetBarrelSlot firstSlot(1, true, "first", 10, 1);
  firstHit.setBarrelSlot( firstSlot );
  
  JPetHit secondHit;
  secondHit.setTime(500);
  JPetBarrelSlot secondSlot(2, true, "second", 30, 2);
  secondHit.setBarrelSlot(secondSlot);
  
  double tof = EventCategorizerTools::calculateTOF(firstHit, secondHit);
  BOOST_REQUIRE_LT( tof, 0 );
}

BOOST_AUTO_TEST_CASE(checkTOFsignPositive)
{
  JPetHit firstHit;
  firstHit.setTime(100);
  JPetBarrelSlot firstSlot(1, true, "first", 30, 1);
  firstHit.setBarrelSlot( firstSlot );
  
  JPetHit secondHit;
  secondHit.setTime(500);
  JPetBarrelSlot secondSlot(2, true, "second", 10, 2);
  secondHit.setBarrelSlot(secondSlot);
  
  double tof = EventCategorizerTools::calculateTOF(firstHit, secondHit);
  BOOST_REQUIRE_GT( tof, 0 );
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AnnihilationPointSuite)

BOOST_AUTO_TEST_CASE(pointAtCenter)
{
  JPetHit firstHit;
  firstHit.setTime(300);
  firstHit.setPos(5,5,0);
  JPetBarrelSlot firstSlot(1, true, "first", 45, 1);
  firstHit.setBarrelSlot( firstSlot );
  
  JPetHit secondHit;
  secondHit.setTime(300);
  secondHit.setPos(-5,-5,0);
  JPetBarrelSlot secondSlot(2, true, "second", 225, 2);
  secondHit.setBarrelSlot(secondSlot);
  
  Point3D point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.x, 0, 0.001);
  BOOST_REQUIRE_CLOSE(point.y, 0, 0.001);
  BOOST_REQUIRE_CLOSE(point.z, 0, 0.001);
}

BOOST_AUTO_TEST_CASE(pointAt0x_5y_0z)
{
  JPetHit firstHit;
  firstHit.setTime(1333/2.0);
  firstHit.setPos(0,45,0);
  JPetBarrelSlot firstSlot(1, true, "first", 90, 1);
  firstHit.setBarrelSlot( firstSlot );
  
  JPetHit secondHit;
  secondHit.setTime(1667/2);
  secondHit.setPos(0,-45,0);
  JPetBarrelSlot secondSlot(2, true, "second", 270, 2);
  secondHit.setBarrelSlot(secondSlot);
  
  Point3D point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.x, 0, 0.1);
  BOOST_REQUIRE_CLOSE(point.y, 5.0, 0.5);
  BOOST_REQUIRE_CLOSE(point.z, 0, 0.1);
}


BOOST_AUTO_TEST_CASE(pointAt0x_m5y_0z)
{
  JPetHit firstHit;
  firstHit.setTime(1333/2.0);
  firstHit.setPos(0,-45,0);
  JPetBarrelSlot firstSlot(1, true, "first", 270, 1);
  firstHit.setBarrelSlot( firstSlot );
  
  JPetHit secondHit;
  secondHit.setTime(1667/2);
  secondHit.setPos(0,45,0);
  JPetBarrelSlot secondSlot(2, true, "second", 90, 2);
  secondHit.setBarrelSlot(secondSlot);
  
  Point3D point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.x, 0, 0.1);
  BOOST_REQUIRE_CLOSE(point.y, -5.0, 0.5);
  BOOST_REQUIRE_CLOSE(point.z, 0, 0.1);
}


BOOST_AUTO_TEST_SUITE_END()
