#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTest
#include <boost/test/unit_test.hpp>


#include <utility>
#include "HitFinderTools.h"
#include <../j-pet-framework/JPetPhysSignal/JPetPhysSignal.h>
#include "../j-pet-framework/JPetLoggerInclude.h"

typedef std::map <int, std::pair < std::vector<JPetPhysSignal>, std::vector<JPetPhysSignal> > > SignalsContainer;


BOOST_AUTO_TEST_SUITE(FirstSuite)

BOOST_AUTO_TEST_CASE( checkZAxisConvention )
{ 
    JPetStatistics stats;
    SignalsContainer allSignalsInTimeWindow;
    std::vector<JPetPhysSignal> signalsA;
    JPetPhysSignal signalA;
    signalA.setTime(1);
    signalsA.push_back(signalA);
    std::vector<JPetPhysSignal> signalsB;
    JPetPhysSignal signalB;
    signalB.setTime(2);
    signalsB.push_back(signalB);
    allSignalsInTimeWindow[99] = std::make_pair<>(signalsA, signalsB);
    double timeDifferenceWindow = 25000;
    std::map<int, std::vector<double>> velMap;
    std::vector<double> values;
    values.push_back(12.6);
    values.push_back(1.1);
    velMap[99]=values;
//     std::vector<JPetHit> hit = HitFinderTools::createHits(stats,allSignalsInTimeWindow, timeDifferenceWindow, velMap);
}

BOOST_AUTO_TEST_SUITE_END()
