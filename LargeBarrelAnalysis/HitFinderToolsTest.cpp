#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTest
#include <boost/test/unit_test.hpp>

#include <utility>
#include "HitFinderTools.h"
#include <JPetPhysSignal/JPetPhysSignal.h>
#include "JPetLoggerInclude.h"

BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE( checkZAxisConvention )
{
  JPetStatistics stats;
  stats.createHistogram( new TH2F("time_diff_per_scin", "time_diff_per_scin" , 100, 0, 100, 100, 0, 100));
  stats.createHistogram( new TH2F("hit_pos_per_scin", "hit_pos_per_scin", 100, 0, 100, 100, 0, 100));

  const int barrelSlotID = 99;

  JPetPhysSignal signalA;
  signalA.setTime(1);
  JPetPM pm1(1);
  JPetBarrelSlot slot(barrelSlotID, true, "", 0, 99);
  pm1.setBarrelSlot(slot);
  signalA.setPM(pm1);

  JPetPhysSignal signalB;
  signalB.setTime(2);
  JPetPM pm2(2);
  signalB.setPM(pm2);

  std::vector<JPetPhysSignal> signalsA;
  signalsA.push_back(signalA);
  std::vector<JPetPhysSignal> signalsB;
  signalsB.push_back(signalB);

  HitFinderTools::SignalsContainer allSignalsInTimeWindow;
  allSignalsInTimeWindow[barrelSlotID] = std::make_pair<>(signalsA, signalsB);

  double timeDifferenceWindow = 25000;

  HitFinderTools::VelocityMap velMap;
  std::pair<double, double> values = std::make_pair(12.6, 1.1);
  velMap[barrelSlotID] = values;

  HitFinderTools HitTools;
  std::vector<JPetHit> hit = HitTools.createHits(stats, allSignalsInTimeWindow, timeDifferenceWindow, velMap);
  BOOST_REQUIRE( hit[0].getPosZ() > 0 );

  allSignalsInTimeWindow.clear();
  signalsA.clear();
  signalA.setTime(5);
  signalsA.push_back(signalA);
  allSignalsInTimeWindow[barrelSlotID] = std::make_pair<>(signalsA, signalsB);
  hit = HitTools.createHits(stats, allSignalsInTimeWindow, timeDifferenceWindow, velMap);
  BOOST_REQUIRE( hit[0].getPosZ() < 0 );
}

BOOST_AUTO_TEST_SUITE_END()
