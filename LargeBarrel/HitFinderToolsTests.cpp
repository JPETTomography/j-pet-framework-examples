#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderTools

#include <boost/test/unit_test.hpp>

#include "HitFinderTools.h"


BOOST_AUTO_TEST_SUITE (HitFinderToolsSuite)

BOOST_AUTO_TEST_CASE (HitFinder)
{

    // All times are in picoseconds

    JPetPhysSignal sig1;
    sig1.setTime(12452);

    JPetPhysSignal sig2;
    sig2.setTime(3146613);

    JPetPhysSignal sig3;
    sig3.setTime(87361353);

    JPetPhysSignal sig4;
    sig4.setTime(46785);

    JPetPhysSignal sig5;
    sig5.setTime(3132169);

    JPetPhysSignal sig6;
    sig6.setTime(87338621);

    HitFinderTools HitFinder;

    std::vector<JPetPhysSignal> SideA = {sig1, sig2, sig3};
    std::vector<JPetPhysSignal> SideB = {sig4, sig5, sig6};

    HitFinderTools::SignalsContainer container;
    container.emplace(7, std::make_pair(SideA, SideB));

    double expectedHitsTimeWindow1ns = 0;
    double expectedHitsTimeWindow50ns = 3;
    double expectedHitsTimeWindow5000ns = 6;
    double expectedHitsTimeWindow1ms = 9;

    double kTimeWindow1ns = pow(10,3);
    double kTimeWindow50ns = 50*pow(10,3);
    double kTimeWindow5000ns = 5000*pow(10,3);
    double kTimeWindow1ms = pow(10,9);

    auto epsilon = 0.1;

    // BOOST_REQUIRE_CLOSE( (double)HitFinder.createHits(container, kTimeWindow1ns).size(), expectedHitsTimeWindow1ns, epsilon);
    BOOST_REQUIRE_CLOSE( (double)HitFinder.createHits(container, kTimeWindow50ns).size(), expectedHitsTimeWindow50ns, epsilon);
    // BOOST_REQUIRE_CLOSE( (double)HitFinder.createHits(container, kTimeWindow5000ns).size(), expectedHitsTimeWindow5000ns, epsilon);
    // BOOST_REQUIRE_CLOSE( (double)HitFinder.createHits(container, kTimeWindow1ms).size(), expectedHitsTimeWindow1ms, epsilon);

}

BOOST_AUTO_TEST_SUITE_END()
