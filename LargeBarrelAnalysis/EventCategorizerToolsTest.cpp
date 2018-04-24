#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE EventCategorizerToolsTests
#include <boost/test/unit_test.hpp>
#include "EventCategorizerTools.h"
#include "JPetSigCh/JPetSigCh.h"

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
  BOOST_REQUIRE(!EventCategorizerTools::checkFor2Gamma(event, stats, false, 3.0));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event0, stats, false, 3.0));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event1, stats, false, 3.0));
  BOOST_REQUIRE(EventCategorizerTools::checkFor2Gamma(event2, stats, false, 3.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkFor2Gamma(event3, stats, false, 3.0));
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

BOOST_AUTO_TEST_CASE(checkForPromptTest_checkTOTCalc)
{
  JPetBarrelSlot barrelSlot(666, true, "Some Slot", 66.0, 666);
  JPetPM pmA(1, "A");
  JPetPM pmB(2, "B");
  pmA.setSide(JPetPM::SideA);
  pmB.setSide(JPetPM::SideB);
  pmA.setBarrelSlot(barrelSlot);
  pmB.setBarrelSlot(barrelSlot);

  JPetSigCh sigCh1;
  JPetSigCh sigCh2;
  JPetSigCh sigCh3;
  JPetSigCh sigCh4;
  JPetSigCh sigCh5;
  JPetSigCh sigCh6;
  JPetSigCh sigCh7;
  JPetSigCh sigCh8;

  sigCh1.setType(JPetSigCh::Leading);
  sigCh2.setType(JPetSigCh::Leading);
  sigCh3.setType(JPetSigCh::Leading);
  sigCh4.setType(JPetSigCh::Leading);
  sigCh5.setType(JPetSigCh::Trailing);
  sigCh6.setType(JPetSigCh::Trailing);
  sigCh7.setType(JPetSigCh::Trailing);
  sigCh8.setType(JPetSigCh::Trailing);

  sigCh1.setThresholdNumber(1);
  sigCh2.setThresholdNumber(2);
  sigCh3.setThresholdNumber(3);
  sigCh4.setThresholdNumber(4);
  sigCh5.setThresholdNumber(1);
  sigCh6.setThresholdNumber(2);
  sigCh7.setThresholdNumber(3);
  sigCh8.setThresholdNumber(4);

  sigCh1.setValue(10.0);
  sigCh2.setValue(12.0);
  sigCh3.setValue(14.0);
  sigCh4.setValue(16.0);
  sigCh5.setValue(23.0);
  sigCh6.setValue(21.0);
  sigCh7.setValue(19.0);
  sigCh8.setValue(17.0);

  JPetRawSignal rawSig1A;
  JPetRawSignal rawSig1B;
  JPetRawSignal rawSig2A;
  JPetRawSignal rawSig2B;
  JPetRawSignal rawSig3A;
  JPetRawSignal rawSig3B;

  rawSig1A.setPM(pmA);
  rawSig1B.setPM(pmB);
  rawSig2A.setPM(pmA);
  rawSig2B.setPM(pmB);
  rawSig3A.setPM(pmA);
  rawSig3B.setPM(pmB);

  rawSig1A.addPoint(sigCh1);
  rawSig1B.addPoint(sigCh1);

  rawSig2A.addPoint(sigCh1);
  rawSig2A.addPoint(sigCh2);
  rawSig2A.addPoint(sigCh3);
  rawSig2A.addPoint(sigCh4);
  rawSig2A.addPoint(sigCh5);
  rawSig2A.addPoint(sigCh6);
  rawSig2A.addPoint(sigCh7);
  rawSig2A.addPoint(sigCh8);

  rawSig2B.addPoint(sigCh1);
  rawSig2B.addPoint(sigCh2);
  rawSig2B.addPoint(sigCh3);
  rawSig2B.addPoint(sigCh4);
  rawSig2B.addPoint(sigCh5);
  rawSig2B.addPoint(sigCh6);
  rawSig2B.addPoint(sigCh7);
  rawSig2B.addPoint(sigCh8);

  sigCh1.setValue(100.0);
  sigCh2.setValue(120.0);
  sigCh3.setValue(140.0);
  sigCh4.setValue(160.0);
  sigCh5.setValue(230.0);
  sigCh6.setValue(210.0);
  sigCh7.setValue(190.0);
  sigCh8.setValue(170.0);

  rawSig3A.addPoint(sigCh1);
  rawSig3A.addPoint(sigCh2);
  rawSig3A.addPoint(sigCh3);
  rawSig3A.addPoint(sigCh4);
  rawSig3A.addPoint(sigCh5);
  rawSig3A.addPoint(sigCh6);
  rawSig3A.addPoint(sigCh7);
  rawSig3A.addPoint(sigCh8);

  rawSig3B.addPoint(sigCh1);
  rawSig3B.addPoint(sigCh2);
  rawSig3B.addPoint(sigCh3);
  rawSig3B.addPoint(sigCh4);
  rawSig3B.addPoint(sigCh5);
  rawSig3B.addPoint(sigCh6);
  rawSig3B.addPoint(sigCh7);
  rawSig3B.addPoint(sigCh8);

 	JPetRecoSignal recoSignal1A;
  JPetRecoSignal recoSignal1B;
  JPetRecoSignal recoSignal2A;
  JPetRecoSignal recoSignal2B;
  JPetRecoSignal recoSignal3A;
  JPetRecoSignal recoSignal3B;

  recoSignal1A.setRawSignal(rawSig1A);
  recoSignal1B.setRawSignal(rawSig1B);
  recoSignal2A.setRawSignal(rawSig2A);
  recoSignal2B.setRawSignal(rawSig2B);
  recoSignal3A.setRawSignal(rawSig3A);
  recoSignal3B.setRawSignal(rawSig3B);

  JPetPhysSignal physSignal1A;
  JPetPhysSignal physSignal1B;
  JPetPhysSignal physSignal2A;
  JPetPhysSignal physSignal2B;
  JPetPhysSignal physSignal3A;
  JPetPhysSignal physSignal3B;

  physSignal1A.setRecoSignal(recoSignal1A);
  physSignal1B.setRecoSignal(recoSignal1B);
  physSignal2A.setRecoSignal(recoSignal2A);
  physSignal2B.setRecoSignal(recoSignal2B);
  physSignal3A.setRecoSignal(recoSignal3A);
  physSignal3B.setRecoSignal(recoSignal3B);

  JPetHit hit1;
  JPetHit hit2;
  JPetHit hit3;

  hit1.setSignals(physSignal1A, physSignal1B);
  hit2.setSignals(physSignal2A, physSignal2B);
  hit3.setSignals(physSignal3A, physSignal3B);

  BOOST_REQUIRE_CLOSE(EventCategorizerTools::calculateTOT(hit1), 0.0, 0.1);
  BOOST_REQUIRE_CLOSE(EventCategorizerTools::calculateTOT(hit2), 56.0, 0.1);
  BOOST_REQUIRE_CLOSE(EventCategorizerTools::calculateTOT(hit3), 560.0, 0.1);

  JPetEvent event1;
  JPetEvent event2;
  JPetEvent event3;
  JPetEvent event4;
  JPetEvent event5;

  event1.addHit(hit1);
  event2.addHit(hit2);
  event3.addHit(hit2);
  event3.addHit(hit3);
  event4.addHit(hit2);
  event4.addHit(hit3);
  event5.addHit(hit1);
  event5.addHit(hit2);
  event5.addHit(hit3);

  JPetStatistics stats;

  BOOST_REQUIRE(!EventCategorizerTools::checkForPrompt(event1, stats, false, 40.0, 60.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkForPrompt(event2, stats, false, 200.0, 400.0));
  BOOST_REQUIRE(!EventCategorizerTools::checkForPrompt(event3, stats, false, 200.0, 400.0));
  BOOST_REQUIRE(EventCategorizerTools::checkForPrompt(event4, stats, false, 40.0, 600.0));
  BOOST_REQUIRE(EventCategorizerTools::checkForPrompt(event5, stats, false, 500.0, 600.0));
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
