#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTest

#include <JPetPhysSignal/JPetPhysSignal.h>
#include <JPetRecoSignal/JPetRecoSignal.h>
#include <JPetRawSignal/JPetRawSignal.h>
#include <boost/test/unit_test.hpp>
#include <JPetSigCh/JPetSigCh.h>
#include "JPetLoggerInclude.h"
#include "HitFinderTools.h"

BOOST_AUTO_TEST_SUITE(HitTestSuite)

BOOST_AUTO_TEST_CASE(getSignalsSlotMap_test_empty)
{
  auto results =  HitFinderTools::getSignalsSlotMap(nullptr);
  BOOST_REQUIRE(results.empty());
}

BOOST_AUTO_TEST_CASE(getSignalsSlotMap_test)
{
  JPetPM pm1(11, "first");
  JPetPM pm2(22, "second");
  JPetPM pm3(33, "third");
  JPetPM pm4(44, "fourth");
  JPetPM pm5(55, "fifth");
  JPetPM pm6(55, "sixth");
  JPetPM pm7(55, "seventh");
  JPetPM pm8(55, "eigth");

  JPetScin scin1(1);
  JPetScin scin2(2);
  JPetScin scin3(3);
  JPetScin scin4(4);
  JPetScin scin5(5);

  pm1.setScin(scin1);
  pm2.setScin(scin1);
  pm3.setScin(scin2);
  pm4.setScin(scin2);
  pm5.setScin(scin3);
  pm6.setScin(scin3);
  pm7.setScin(scin4);
  pm8.setScin(scin5);

  JPetPhysSignal physSig1, physSig2, physSig3, physSig4;
  JPetPhysSignal physSig5, physSig6, physSig7, physSig8;
  physSig1.setPM(pm1);
  physSig2.setPM(pm2);
  physSig3.setPM(pm3);
  physSig4.setPM(pm4);
  physSig5.setPM(pm5);
  physSig6.setPM(pm6);
  physSig7.setPM(pm7);
  physSig8.setPM(pm8);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSig1);
  slot.add<JPetPhysSignal>(physSig2);
  slot.add<JPetPhysSignal>(physSig3);
  slot.add<JPetPhysSignal>(physSig4);
  slot.add<JPetPhysSignal>(physSig5);
  slot.add<JPetPhysSignal>(physSig6);
  slot.add<JPetPhysSignal>(physSig7);
  slot.add<JPetPhysSignal>(physSig8);

  auto results = HitFinderTools::getSignalsSlotMap(&slot);
  BOOST_REQUIRE_EQUAL(results.size(), 5);
  BOOST_REQUIRE(results.find(1) != results.end());
  BOOST_REQUIRE(results.find(2) != results.end());
  BOOST_REQUIRE(results.find(3) != results.end());
  BOOST_REQUIRE(results.find(4) != results.end());
  BOOST_REQUIRE(results.find(5) != results.end());
  BOOST_REQUIRE_EQUAL(results.at(1).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(2).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(3).size(), 2);
  BOOST_REQUIRE_EQUAL(results.at(4).size(), 1);
  BOOST_REQUIRE_EQUAL(results.at(5).size(), 1);
}

BOOST_AUTO_TEST_CASE(matchSignals_test_allSigsSameSide)
{
  JPetPM pm1(11, "first");
  JPetPM pm2(22, "second");
  JPetPM pm3(33, "third");

  JPetScin scin1(1);
  JPetScin scin2(2);

  pm1.setScin(scin1);
  pm1.setSide(JPetPM::SideA);
  pm2.setScin(scin2);
  pm3.setScin(scin2);
  pm2.setSide(JPetPM::SideA);
  pm3.setSide(JPetPM::SideA);

  JPetPhysSignal physSig11, physSig12, physSig13;
  JPetPhysSignal physSig21, physSig22, physSig23;
  JPetPhysSignal physSig31, physSig32, physSig33;

  physSig11.setPM(pm1);
  physSig12.setPM(pm1);
  physSig13.setPM(pm1);
  physSig21.setPM(pm2);
  physSig22.setPM(pm2);
  physSig23.setPM(pm2);
  physSig31.setPM(pm3);
  physSig32.setPM(pm3);
  physSig33.setPM(pm3);

  physSig11.setTime(1.0);
  physSig12.setTime(2.0);
  physSig13.setTime(3.0);
  physSig21.setTime(1.1);
  physSig22.setTime(2.1);
  physSig23.setTime(3.1);
  physSig31.setTime(1.2);
  physSig32.setTime(2.2);
  physSig33.setTime(3.3);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSig11);
  slot.add<JPetPhysSignal>(physSig12);
  slot.add<JPetPhysSignal>(physSig13);
  slot.add<JPetPhysSignal>(physSig21);
  slot.add<JPetPhysSignal>(physSig22);
  slot.add<JPetPhysSignal>(physSig23);
  slot.add<JPetPhysSignal>(physSig31);
  slot.add<JPetPhysSignal>(physSig32);
  slot.add<JPetPhysSignal>(physSig33);

  auto signalSlotMap = HitFinderTools::getSignalsSlotMap(&slot);
  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  double timeDiffAB = 10.0;

  std::vector<JPetHit> allHits = HitFinderTools::matchSignals(
    stats, signalSlotMap, velocitiesMap, timeDiffAB, -1, false);

  BOOST_REQUIRE(allHits.empty());
}

BOOST_AUTO_TEST_CASE(matchSignals_test_smallTimeDiff)
{
  JPetPM pm1(11, "first");
  JPetPM pm2(22, "second");
  JPetPM pm3(33, "third");
  JPetPM pm4(44, "fourth");

  JPetScin scin1(1);
  JPetScin scin2(2);

  pm1.setScin(scin1);
  pm2.setScin(scin1);
  pm1.setSide(JPetPM::SideA);
  pm2.setSide(JPetPM::SideB);
  pm3.setScin(scin2);
  pm4.setScin(scin2);
  pm3.setSide(JPetPM::SideA);
  pm4.setSide(JPetPM::SideA);

  JPetPhysSignal physSig11, physSig12, physSig13;
  JPetPhysSignal physSig21, physSig22, physSig23;
  JPetPhysSignal physSig31, physSig32, physSig33;
  JPetPhysSignal physSig41, physSig42, physSig43;

  physSig11.setPM(pm1);
  physSig12.setPM(pm1);
  physSig13.setPM(pm1);
  physSig21.setPM(pm2);
  physSig22.setPM(pm2);
  physSig23.setPM(pm2);
  physSig31.setPM(pm3);
  physSig32.setPM(pm3);
  physSig33.setPM(pm3);
  physSig41.setPM(pm4);
  physSig42.setPM(pm4);
  physSig43.setPM(pm4);

  physSig11.setTime(10.0);
  physSig12.setTime(20.0);
  physSig13.setTime(30.0);
  physSig21.setTime(11.0);
  physSig22.setTime(22.0);
  physSig23.setTime(33.0);
  physSig31.setTime(10.0);
  physSig32.setTime(11.0);
  physSig33.setTime(12.0);
  physSig41.setTime(10.5);
  physSig42.setTime(11.5);
  physSig43.setTime(12.5);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSig11);
  slot.add<JPetPhysSignal>(physSig12);
  slot.add<JPetPhysSignal>(physSig13);
  slot.add<JPetPhysSignal>(physSig21);
  slot.add<JPetPhysSignal>(physSig22);
  slot.add<JPetPhysSignal>(physSig23);
  slot.add<JPetPhysSignal>(physSig31);
  slot.add<JPetPhysSignal>(physSig32);
  slot.add<JPetPhysSignal>(physSig33);

  auto signalSlotMap = HitFinderTools::getSignalsSlotMap(&slot);
  JPetStatistics stats;
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  double timeDiffAB = 0.001;

  std::vector<JPetHit> allHits = HitFinderTools::matchSignals(
    stats, signalSlotMap, velocitiesMap, timeDiffAB, -1, false);

  BOOST_REQUIRE(allHits.empty());
}

BOOST_AUTO_TEST_CASE(matchSignals_test_refDetSigs)
{
  JPetBarrelSlot refBarrelSlot(193, true, "refBS", 45.0, 193);
  JPetScin refScin(999);
  JPetPM refPm(11, "ref");
  refPm.setScin(refScin);
  refPm.setBarrelSlot(refBarrelSlot);

  JPetPhysSignal physSig1, physSig2, physSig3;
  physSig1.setPM(refPm);
  physSig2.setPM(refPm);
  physSig3.setPM(refPm);
  physSig1.setTime(1.0);
  physSig2.setTime(2.0);
  physSig3.setTime(3.0);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSig1);
  slot.add<JPetPhysSignal>(physSig2);
  slot.add<JPetPhysSignal>(physSig3);

  auto signalSlotMap = HitFinderTools::getSignalsSlotMap(&slot);
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  double timeDiffAB = 1.1;
  JPetStatistics stats;
  std::vector<JPetHit> allHits = HitFinderTools::matchSignals(
    stats, signalSlotMap, velocitiesMap, timeDiffAB, 999, false);

  BOOST_REQUIRE_EQUAL(allHits.size(), 3);
}

BOOST_AUTO_TEST_CASE(matchSignals_test)
{
  JPetLayer layer1(1, true, "layer1", 10.0);
  JPetBarrelSlot barrelSlot1(23, true, "barel1", 30.0, 23);
  barrelSlot1.setLayer(layer1);

  JPetScin scin1(23);
  JPetPM pm1A(31, "1A");
  JPetPM pm1B(75, "1B");
  pm1A.setScin(scin1);
  pm1B.setScin(scin1);
  pm1A.setBarrelSlot(barrelSlot1);
  pm1B.setBarrelSlot(barrelSlot1);
  pm1A.setSide(JPetPM::SideA);
  pm1B.setSide(JPetPM::SideB);

  JPetTOMBChannel channel1(66);
  JPetTOMBChannel channel2(88);
  JPetSigCh *sigCh1 = new JPetSigCh();
  JPetSigCh *sigCh2 = new JPetSigCh();
  sigCh1->setTOMBChannel(channel1);
  sigCh2->setTOMBChannel(channel2);
  sigCh1->setThresholdNumber(1);
  sigCh2->setThresholdNumber(1);
  sigCh1->setPM(pm1A);
  sigCh2->setPM(pm1B);

  JPetRawSignal *raw1 = new JPetRawSignal();
  JPetRawSignal *raw2 = new JPetRawSignal();
  raw1->addPoint(*sigCh1);
  raw2->addPoint(*sigCh2);

  JPetRecoSignal *reco1 = new JPetRecoSignal();
  JPetRecoSignal *reco2 = new JPetRecoSignal();
  reco1->setRawSignal(*raw1);
  reco2->setRawSignal(*raw2);

  JPetPhysSignal physSig1A, physSig1B;
  JPetPhysSignal physSig2A, physSig2B;
  JPetPhysSignal physSig3A, physSig3B;
  JPetPhysSignal physSigA;
  physSigA.setRecoSignal(*reco1);
  physSig1A.setRecoSignal(*reco1);
  physSig2A.setRecoSignal(*reco1);
  physSig3A.setRecoSignal(*reco1);
  physSig1B.setRecoSignal(*reco2);
  physSig2B.setRecoSignal(*reco2);
  physSig3B.setRecoSignal(*reco2);
  physSigA.setPM(pm1A);
  physSig1A.setPM(pm1A);
  physSig1B.setPM(pm1B);
  physSig2A.setPM(pm1A);
  physSig2B.setPM(pm1B);
  physSig3A.setPM(pm1A);
  physSig3B.setPM(pm1B);
  physSig1A.setTime(1.0);
  physSig1B.setTime(1.5);
  physSigA.setTime(2.2);
  physSig2A.setTime(4.0);
  physSig2B.setTime(4.8);
  physSig3A.setTime(7.2);
  physSig3B.setTime(6.5);

  JPetTimeWindow slot("JPetPhysSignal");
  slot.add<JPetPhysSignal>(physSigA);
  slot.add<JPetPhysSignal>(physSig1A);
  slot.add<JPetPhysSignal>(physSig1B);
  slot.add<JPetPhysSignal>(physSig2A);
  slot.add<JPetPhysSignal>(physSig2B);
  slot.add<JPetPhysSignal>(physSig3A);
  slot.add<JPetPhysSignal>(physSig3B);

  auto signalSlotMap = HitFinderTools::getSignalsSlotMap(&slot);
  std::map<unsigned int, std::vector<double>> velocitiesMap;
  std::vector<double> velVec = {2.0, 3.4, 4.5, 5.6};
  std::pair<unsigned int, std::vector<double>> pair1 = std::make_pair(66, velVec);
  std::pair<unsigned int, std::vector<double>> pair2 = std::make_pair(88, velVec);
  velocitiesMap.insert(pair1);
  velocitiesMap.insert(pair2);

  double timeDiffAB = 1.0;
  auto epsilon = 0.0001;
  JPetStatistics stats;
  std::vector<JPetHit> allHits = HitFinderTools::matchSignals(
    stats, signalSlotMap, velocitiesMap, timeDiffAB, -1, false);

  BOOST_REQUIRE_EQUAL(allHits.size(), 3);

  BOOST_REQUIRE_EQUAL(allHits.at(0).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(allHits.at(0).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(allHits.at(1).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(allHits.at(1).getSignalB().getPM().getID(), 75);
  BOOST_REQUIRE_EQUAL(allHits.at(2).getSignalA().getPM().getID(), 31);
  BOOST_REQUIRE_EQUAL(allHits.at(2).getSignalB().getPM().getID(), 75);

  BOOST_REQUIRE_EQUAL(allHits.at(0).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(allHits.at(0).getSignalB().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(allHits.at(1).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(allHits.at(1).getSignalB().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(allHits.at(2).getSignalA().getPM().getScin().getID(), 23);
  BOOST_REQUIRE_EQUAL(allHits.at(2).getSignalB().getPM().getScin().getID(), 23);

  BOOST_REQUIRE_CLOSE(allHits.at(0).getTime(), (1.0+1.5)/2, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(1).getTime(), (4.0+4.8)/2, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(2).getTime(), (6.5+7.2)/2, epsilon);

  BOOST_REQUIRE_CLOSE(allHits.at(0).getTimeDiff(), 1.5-1.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(1).getTimeDiff(), 4.8-4.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(2).getTimeDiff(), 6.5-7.2, epsilon);

  BOOST_REQUIRE_CLOSE(allHits.at(0).getPosX(), 8.660254038, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(1).getPosX(), 8.660254038, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(2).getPosX(), 8.660254038, epsilon);

  BOOST_REQUIRE_CLOSE(allHits.at(0).getPosY(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(1).getPosY(), 5.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(2).getPosY(), 5.0, epsilon);

  BOOST_REQUIRE_CLOSE(allHits.at(0).getPosZ(), 2.0 * allHits.at(0).getTimeDiff() / 2000.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(1).getPosZ(), 2.0 * allHits.at(1).getTimeDiff() / 2000.0, epsilon);
  BOOST_REQUIRE_CLOSE(allHits.at(2).getPosZ(), 2.0 * allHits.at(2).getTimeDiff() / 2000.0, epsilon);

  BOOST_REQUIRE(allHits.at(0).getPosZ()>0.0);
  BOOST_REQUIRE(allHits.at(1).getPosZ()>0.0);
  BOOST_REQUIRE(allHits.at(2).getPosZ()<0.0);
}

BOOST_AUTO_TEST_CASE(sortByTime_test)
{
  JPetPhysSignal physSig1, physSig2, physSig3, physSig4, physSig5;
  physSig1.setTime(1.0);
  physSig2.setTime(2.0);
  physSig3.setTime(3.0);
  physSig4.setTime(4.0);
  physSig5.setTime(5.0);

  std::vector<JPetPhysSignal> orderedVector;
  std::vector<JPetPhysSignal> unorderedVector;

  orderedVector.push_back(physSig1);
  orderedVector.push_back(physSig2);
  orderedVector.push_back(physSig3);
  orderedVector.push_back(physSig4);
  orderedVector.push_back(physSig5);

  unorderedVector.push_back(physSig2);
  unorderedVector.push_back(physSig5);
  unorderedVector.push_back(physSig1);
  unorderedVector.push_back(physSig4);
  unorderedVector.push_back(physSig3);

  HitFinderTools::sortByTime(unorderedVector);

  std::vector<double> timesOrderedVector;
  std::vector<double> timesUnorderedVector;
  for(auto sig1 : orderedVector)
    timesOrderedVector.push_back(sig1.getTime());
  for(auto sig2 : unorderedVector)
    timesUnorderedVector.push_back(sig2.getTime());

  BOOST_CHECK_EQUAL_COLLECTIONS(
    timesUnorderedVector.begin(), timesUnorderedVector.end(),
    timesOrderedVector.begin(), timesOrderedVector.end());
}

BOOST_AUTO_TEST_SUITE_END()
