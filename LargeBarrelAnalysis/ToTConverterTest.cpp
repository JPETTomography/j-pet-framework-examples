#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ToTConverterTest

#include <boost/test/unit_test.hpp>
#include "JPetLoggerInclude.h"
#include "ToTConverter.h"

BOOST_AUTO_TEST_SUITE(ToTConverterTestSuite)

BOOST_AUTO_TEST_CASE(getTot_test)
{
  ///This test is stupid, but I don't know for a moment what values to test.
  BOOST_CHECK_CLOSE(ToTConverter::getToT(0), 0, 0.1);
}

BOOST_AUTO_TEST_SUITE_END()
