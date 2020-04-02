#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ToTEnergyConverterTest

#include <boost/test/unit_test.hpp>
#include "JPetLoggerInclude.h"
#include "../ToTEnergyConverter.h"
using namespace jpet_common_tools;

/// Returns Time-over-threshold for given deposited energy
/// the current parametrization is par1 + par2 * eDep
/// Returned value in ps, and eDep is given in keV.
double getToT1(double eDep, double  par1 = -91958, double par2 = 19341)
{
  if (eDep < 0 ) return 0;
  double value = par1 + eDep * par2;
  return value;
}

BOOST_AUTO_TEST_SUITE(ToTConverterTestSuite)

BOOST_AUTO_TEST_CASE(getTot_standardFunc)
{
  JPetCachedFunctionParams params("pol1", {-91958, 19341});
  ToTEnergyConverter conv(params, Range(10000, 0., 100.));
  BOOST_CHECK_CLOSE(conv(0), getToT1(0), 0.1);
  BOOST_CHECK_CLOSE(conv(1), getToT1(1), 0.1);
  BOOST_CHECK_CLOSE(conv(10), getToT1(10), 0.1);
  BOOST_CHECK_CLOSE(conv(59.5), getToT1(59.5), 0.1);
  BOOST_CHECK_CLOSE(conv(99.9), getToT1(99.9), 0.1);
}

BOOST_AUTO_TEST_SUITE_END()
