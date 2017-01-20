#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE SignalFinderToolsTest
#include <boost/test/unit_test.hpp>

#include "SignalFinderTools.h"


BOOST_AUTO_TEST_SUITE(FirstSuite)
BOOST_AUTO_TEST_CASE( buildRawSignals )
{
 JPetStatistics stats;
 auto sigCh1 =JPetSigCh(JPetSigCh::Leading, 10);
 sigCh1.setThreshold(1);
    
 std::vector<JPetSigCh> sigChFromSamePM = {sigCh1};
 auto results =  SignalFinderTools::buildRawSignals(4, sigChFromSamePM, 1, stats, false, 5,5);
 BOOST_REQUIRE_EQUAL(results.size(), 1);
 //BOOST_REQUIRE_EQUAL(results[0].getTimeWindowIndex(), 4);
}

BOOST_AUTO_TEST_SUITE_END()
