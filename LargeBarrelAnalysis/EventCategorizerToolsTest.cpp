#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EventCategorizerToolsTests
#include <boost/test/unit_test.hpp>
#include "EventCategorizerTools.h"
#include "JPetHit/JPetHit.h"

BOOST_AUTO_TEST_SUITE(CategoryCheckSuite)

BOOST_AUTO_TEST_CASE(checkFor2GammaTest)
{
  JPetBarrelSlot firstSlot(1, true, "first", 10.0, 1);
  JPetBarrelSlot secondSlot(2, true, "second", 190.0, 2);
  JPetBarrelSlot thirdSlot(3, true, "second", 45.5, 3);
  JPetBarrelSlot fourthSlot(4, true, "second", 226.25, 4);

  JPetHit firstHit;
  JPetHit secondHit;
  JPetHit thirdHit;
  JPetHit fourthHit;

  firstHit.setBarrelSlot(firstSlot);
  secondHit.setBarrelSlot(secondSlot);
  thirdHit.setBarrelSlot(thirdSlot);
  fourthHit.setBarrelSlot(fourthSlot);

  JPetEvent event;
  event.addHit(thirdHit);

  JPetEvent event0;
  event0.addHit(firstHit);
  event0.addHit(secondHit);
  event0.addHit(thirdHit);
  event0.addHit(fourthHit);

  JPetEvent event1;
  event1.addHit(firstHit);
  event1.addHit(secondHit);

  JPetEvent event2;
  event2.addHit(thirdHit);
  event2.addHit(fourthHit);

  JPetEvent event3;
  event3.addHit(secondHit);
  event3.addHit(fourthHit);

  JPetStatistics stats;
  BOOST_REQUIRE(!EventCategorizerTools::checkFor2Gamma(event, stats, false));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event0, stats, false));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event1, stats, false));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event2, stats, false));
  BOOST_REQUIRE(!EventCategorizerTools::checkFor2Gamma(event3, stats, false));
}

BOOST_AUTO_TEST_CASE(checkFor3GammaTest)
{
  JPetBarrelSlot firstSlot(1, true, "first", 10.0, 1);
  JPetBarrelSlot secondSlot(2, true, "second", 190.0, 2);
  JPetBarrelSlot thirdSlot(3, true, "second", 45.5, 3);
  JPetBarrelSlot fourthSlot(4, true, "second", 226.25, 4);

  JPetHit firstHit;
  JPetHit secondHit;
  JPetHit thirdHit;
  JPetHit fourthHit;

  firstHit.setBarrelSlot(firstSlot);
  secondHit.setBarrelSlot(secondSlot);
  thirdHit.setBarrelSlot(thirdSlot);
  fourthHit.setBarrelSlot(fourthSlot);

  JPetEvent event0;
  event0.addHit(firstHit);

  JPetEvent event1;
  event1.addHit(firstHit);
  event1.addHit(secondHit);

  JPetEvent event2;
  event2.addHit(firstHit);
  event2.addHit(secondHit);
  event2.addHit(thirdHit);

  JPetEvent event3;
  event3.addHit(firstHit);
  event3.addHit(secondHit);
  event3.addHit(thirdHit);
  event3.addHit(fourthHit);

  JPetStatistics stats;
  BOOST_REQUIRE(!EventCategorizerTools::checkFor3Gamma(event0, stats, false));
  BOOST_REQUIRE(!EventCategorizerTools::checkFor3Gamma(event1, stats, false));
  BOOST_REQUIRE(EventCategorizerTools::checkFor3Gamma(event2, stats, false));
  BOOST_REQUIRE(EventCategorizerTools::checkFor3Gamma(event3, stats, false));

}

BOOST_AUTO_TEST_CASE(checkForPromptTest)
{
  JPetEvent event;
  JPetStatistics stats;
  BOOST_REQUIRE(!EventCategorizerTools::checkForPrompt(event, stats, false));
}

BOOST_AUTO_TEST_CASE(checkForScatterTest)
{
  JPetHit firstHit;
  JPetHit secondHit;
  firstHit.setTime(25.7);
  secondHit.setTime(25.2);
  firstHit.setPos(10000.0,10000.0,10000.0);
  secondHit.setPos(-10000.0,-10000.0,-10000.0);

  JPetEvent event;
  event.addHit(firstHit);
  event.addHit(secondHit);

  JPetEvent event1;
  event1.addHit(firstHit);

  JPetStatistics stats;
  BOOST_REQUIRE(EventCategorizerTools::checkForScatter(event, stats, false, 1.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkForScatter(event, stats, false, 0.000001));
  BOOST_REQUIRE(!EventCategorizerTools::checkForScatter(event1, stats, false, 1.0));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(TOFSuite)

BOOST_AUTO_TEST_CASE(checkHitOrder)
{
  JPetHit firstHit;
  JPetHit secondHit;
  firstHit.setTime(500.0);
  secondHit.setTime(100.0);

  BOOST_REQUIRE_CLOSE(EventCategorizerTools::calculateTOF(firstHit, secondHit), kUndefinedValue, 0.100);
}

BOOST_AUTO_TEST_CASE(checkTOFsignNegative)
{
  JPetBarrelSlot firstSlot(1, true, "first", 10, 1);
  JPetBarrelSlot secondSlot(2, true, "second", 30, 2);
  JPetHit firstHit;
  JPetHit secondHit;
  firstHit.setTime(100.0);
  secondHit.setTime(500.0);
  firstHit.setBarrelSlot(firstSlot);
  secondHit.setBarrelSlot(secondSlot);

  BOOST_REQUIRE_LT(EventCategorizerTools::calculateTOF(firstHit, secondHit), 0);
}

BOOST_AUTO_TEST_CASE(checkTOFsignPositive)
{
  JPetBarrelSlot firstSlot(1, true, "first", 30, 1);
  JPetBarrelSlot secondSlot(2, true, "second", 10, 2);
  JPetHit firstHit;
  JPetHit secondHit;
  firstHit.setTime(100.0);
  secondHit.setTime(500.0);
  firstHit.setBarrelSlot(firstSlot);
  secondHit.setBarrelSlot(secondSlot);

  BOOST_REQUIRE_GT(EventCategorizerTools::calculateTOF(firstHit, secondHit), 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(AnnihilationPointSuite)

BOOST_AUTO_TEST_CASE(pointAtCenter)
{
  JPetBarrelSlot firstSlot(1, true, "first", 45, 1);
  JPetBarrelSlot secondSlot(2, true, "second", 225, 2);
  JPetHit firstHit;
  JPetHit secondHit;
  firstHit.setTime(300.0);
  secondHit.setTime(300.0);
  firstHit.setPos(5.0,5.0,0.0);
  secondHit.setPos(-5.0,-5.0,0.0);
  firstHit.setBarrelSlot(firstSlot);
  secondHit.setBarrelSlot(secondSlot);
  TVector3 point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.X(), 0, 0.001);
  BOOST_REQUIRE_CLOSE(point.Y(), 0, 0.001);
  BOOST_REQUIRE_CLOSE(point.Z(), 0, 0.001);
}

BOOST_AUTO_TEST_CASE(pointAt0x_5y_0z)
{
  JPetHit firstHit;
  firstHit.setTime(1333.0/2.0);
  firstHit.setPos(0.0,45.0,0.0);
  JPetBarrelSlot firstSlot(1, true, "first", 90, 1);
  firstHit.setBarrelSlot( firstSlot );

  JPetHit secondHit;
  secondHit.setTime(1667.0/2.0);
  secondHit.setPos(0.0,-45.0,0.0);
  JPetBarrelSlot secondSlot(2, true, "second", 270, 2);
  secondHit.setBarrelSlot(secondSlot);

  TVector3 point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.X(), 0.0, 0.1);
  BOOST_REQUIRE_CLOSE(point.Y(), 5.0, 0.5);
  BOOST_REQUIRE_CLOSE(point.Z(), 0.0, 0.1);
}

BOOST_AUTO_TEST_CASE(pointAt0x_m5y_0z)
{
  JPetHit firstHit;
  firstHit.setTime(1333.0/2.0);
  firstHit.setPos(0.0,-45.0,0.0);
  JPetBarrelSlot firstSlot(1, true, "first", 270, 1);
  firstHit.setBarrelSlot( firstSlot );

  JPetHit secondHit;
  secondHit.setTime(1667.0/2.0);
  secondHit.setPos(0.0,45.0,0.0);
  JPetBarrelSlot secondSlot(2, true, "second", 90, 2);
  secondHit.setBarrelSlot(secondSlot);

  TVector3 point = EventCategorizerTools::calculateAnnihilationPoint(firstHit, secondHit);
  BOOST_REQUIRE_CLOSE(point.X(), 0.0, 0.1);
  BOOST_REQUIRE_CLOSE(point.Y(), -5.0, 0.5);
  BOOST_REQUIRE_CLOSE(point.Z(), 0.0, 0.1);
}

BOOST_AUTO_TEST_SUITE_END()
