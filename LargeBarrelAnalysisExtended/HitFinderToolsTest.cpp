#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTests

#include <boost/test/unit_test.hpp>

#include "HitFinderTools.h"


BOOST_AUTO_TEST_SUITE (HitFinderToolsSuite)

BOOST_AUTO_TEST_CASE (HitFinderOneScintOneTimeSlot)
{

    // All times are in picoseconds

    JPetBarrelSlot BS(7, true, "string", 1.0, 1);
    JPetScin Scintillator(1);

    JPetPhysSignal sig1;
    sig1.setTime(12452);
    sig1.setTimeWindowIndex(1);

    JPetPM PM1;
    PM1.setSide(JPetPM::SideA);
    PM1.setBarrelSlot(BS);
    PM1.setScin(Scintillator);
    sig1.setPM(PM1);


    JPetPhysSignal sig2;
    sig2.setTime(3146613);
    sig2.setTimeWindowIndex(1);

    JPetPM PM2;
    PM2.setSide(JPetPM::SideA);
    PM2.setBarrelSlot(BS);
    PM2.setScin(Scintillator);
    sig2.setPM(PM2);

    JPetPhysSignal sig3;
    sig3.setTime(87361353);
    sig3.setTimeWindowIndex(1);

    JPetPM PM3;
    PM3.setSide(JPetPM::SideA);
    PM3.setBarrelSlot(BS);
    PM3.setScin(Scintillator);
    sig3.setPM(PM3);

    JPetPhysSignal sig4;
    sig4.setTime(46785);
    sig4.setTimeWindowIndex(1);

    JPetPM PM4;
    PM4.setSide(JPetPM::SideB);
    PM4.setBarrelSlot(BS);
    PM4.setScin(Scintillator);
    sig4.setPM(PM4);

    JPetPhysSignal sig5;
    sig5.setTime(3132169);
    sig5.setTimeWindowIndex(1);

    JPetPM PM5;
    PM5.setSide(JPetPM::SideB);
    PM5.setBarrelSlot(BS);
    PM5.setScin(Scintillator);
    sig5.setPM(PM5);

    JPetPhysSignal sig6;
    sig6.setTime(87338621);
    sig6.setTimeWindowIndex(1);

    JPetPM PM6;
    PM6.setSide(JPetPM::SideB);
    PM6.setBarrelSlot(BS);
    PM6.setScin(Scintillator);
    sig6.setPM(PM6);




    HitFinderTools HitFinder;

    std::vector<JPetPhysSignal> side7A = {sig1, sig2, sig3};
    std::vector<JPetPhysSignal> side7B = {sig4, sig5, sig6};


    HitFinderTools::SignalsContainer container;

    container.emplace(7, std::make_pair(side7A, side7B));


    double expectedHitsTimeWindow1ns = 0;
    double expectedHitsTimeWindow50ns = 3;
    double expectedHitsTimeWindow5000ns = 5;
    double expectedHitsTimeWindow1ms = 9;

    double kTimeWindow1ns = pow(10,3);
    double kTimeWindow50ns = 50*pow(10,3);
    double kTimeWindow5000ns = 5000*pow(10,3);
    double kTimeWindow1ms = pow(10,9);

    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow1ns).size(), expectedHitsTimeWindow1ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow50ns).size(), expectedHitsTimeWindow50ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow5000ns).size(), expectedHitsTimeWindow5000ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow1ms).size(), expectedHitsTimeWindow1ms);

}

BOOST_AUTO_TEST_CASE (HitFinderTwoScintOneTimeSlot)
{

    // All times are in picoseconds

    JPetBarrelSlot BS(7, true, "string", 1.0, 1);
    JPetScin Scintillator(1);

    JPetPhysSignal sig1;
    sig1.setTime(12452);
    sig1.setTimeWindowIndex(1);

    JPetPM PM1;
    PM1.setSide(JPetPM::SideA);
    PM1.setBarrelSlot(BS);
    PM1.setScin(Scintillator);
    sig1.setPM(PM1);


    JPetPhysSignal sig2;
    sig2.setTime(3146613);
    sig2.setTimeWindowIndex(1);

    JPetPM PM2;
    PM2.setSide(JPetPM::SideA);
    PM2.setBarrelSlot(BS);
    PM2.setScin(Scintillator);
    sig2.setPM(PM2);

    JPetPhysSignal sig3;
    sig3.setTime(87361353);
    sig3.setTimeWindowIndex(1);

    JPetPM PM3;
    PM3.setSide(JPetPM::SideA);
    PM3.setBarrelSlot(BS);
    PM3.setScin(Scintillator);
    sig3.setPM(PM3);

    JPetPhysSignal sig4;
    sig4.setTime(46785);
    sig4.setTimeWindowIndex(1);

    JPetPM PM4;
    PM4.setSide(JPetPM::SideB);
    PM4.setBarrelSlot(BS);
    PM4.setScin(Scintillator);
    sig4.setPM(PM4);

    JPetPhysSignal sig5;
    sig5.setTime(3132169);
    sig5.setTimeWindowIndex(1);

    JPetPM PM5;
    PM5.setSide(JPetPM::SideB);
    PM5.setBarrelSlot(BS);
    PM5.setScin(Scintillator);
    sig5.setPM(PM5);

    JPetPhysSignal sig6;
    sig6.setTime(87338621);
    sig6.setTimeWindowIndex(1);

    JPetPM PM6;
    PM6.setSide(JPetPM::SideB);
    PM6.setBarrelSlot(BS);
    PM6.setScin(Scintillator);
    sig6.setPM(PM6);



    // SECOND SCINTILLATOR

    JPetBarrelSlot BS9(9, true, "string2", 10.0, 1);
    JPetScin Scintillator9(10);

    JPetPhysSignal sig91;
    sig1.setTime(13452);
    sig1.setTimeWindowIndex(1);

    JPetPM PM91;
    PM91.setSide(JPetPM::SideA);
    PM91.setBarrelSlot(BS9);
    PM91.setScin(Scintillator9);
    sig91.setPM(PM91);


    JPetPhysSignal sig92;
    sig92.setTime(3147613);
    sig92.setTimeWindowIndex(1);

    JPetPM PM92;
    PM92.setSide(JPetPM::SideA);
    PM92.setBarrelSlot(BS9);
    PM92.setScin(Scintillator9);
    sig92.setPM(PM92);

    JPetPhysSignal sig93;
    sig93.setTime(87362353);
    sig93.setTimeWindowIndex(1);

    JPetPM PM93;
    PM93.setSide(JPetPM::SideA);
    PM93.setBarrelSlot(BS9);
    PM93.setScin(Scintillator9);
    sig93.setPM(PM93);

    JPetPhysSignal sig94;
    sig94.setTime(47785);
    sig94.setTimeWindowIndex(1);

    JPetPM PM94;
    PM94.setSide(JPetPM::SideB);
    PM94.setBarrelSlot(BS9);
    PM94.setScin(Scintillator9);
    sig94.setPM(PM94);

    JPetPhysSignal sig95;
    sig95.setTime(3133169);
    sig95.setTimeWindowIndex(1);

    JPetPM PM95;
    PM95.setSide(JPetPM::SideB);
    PM95.setBarrelSlot(BS9);
    PM95.setScin(Scintillator9);
    sig95.setPM(PM95);

    JPetPhysSignal sig96;
    sig96.setTime(87339621);
    sig96.setTimeWindowIndex(1);

    JPetPM PM96;
    PM96.setSide(JPetPM::SideB);
    PM96.setBarrelSlot(BS9);
    PM96.setScin(Scintillator9);
    sig96.setPM(PM96);




    HitFinderTools HitFinder;

    std::vector<JPetPhysSignal> side7A = {sig1, sig2, sig3};
    std::vector<JPetPhysSignal> side7B = {sig4, sig5, sig6};

    std::vector<JPetPhysSignal> side9A = {sig91, sig92, sig93};
    std::vector<JPetPhysSignal> side9B = {sig94, sig95, sig96};

    HitFinderTools::SignalsContainer container;

    container.emplace(7, std::make_pair(side7A, side7B));
    container.emplace(9, std::make_pair(side9A, side9B));


    double expectedHitsTimeWindow1ns = 0;
    double expectedHitsTimeWindow50ns = 6;
    double expectedHitsTimeWindow5000ns = 10;
    double expectedHitsTimeWindow1ms = 18;

    double kTimeWindow1ns = pow(10,3);
    double kTimeWindow50ns = 50*pow(10,3);
    double kTimeWindow5000ns = 5000*pow(10,3);
    double kTimeWindow1ms = pow(10,9);



    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow1ns).size(), expectedHitsTimeWindow1ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow50ns).size(), expectedHitsTimeWindow50ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow5000ns).size(), expectedHitsTimeWindow5000ns);
    BOOST_REQUIRE_EQUAL(HitFinder.createHits(container, kTimeWindow1ms).size(), expectedHitsTimeWindow1ms);

}

BOOST_AUTO_TEST_SUITE_END()
