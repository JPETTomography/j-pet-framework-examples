#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ToTConverterTest

#include <boost/test/unit_test.hpp>
#include "JPetLoggerInclude.h"
#include "ToTConverter.h"

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

BOOST_AUTO_TEST_CASE(getTot_params)
{
  ToTConverterParams params("pol1", { -91958, 19341}, 100, 0., 100.);
  ToTConverter conv(params);
  BOOST_CHECK(conv.getParams().fValidFunction);
  BOOST_CHECK_EQUAL(conv.getParams().fParams.size(), 2);
  BOOST_CHECK_EQUAL(conv.getParams().fParams[0], -91958);
  BOOST_CHECK_EQUAL(conv.getParams().fParams[1], 19341);
  BOOST_CHECK_EQUAL(conv.getValues().size(), 100);
  auto vals = conv.getValues();
}

BOOST_AUTO_TEST_CASE(getTot_standardFunc)
{
  ToTConverterParams params("pol1", { -91958, 19341}, 10000, 0., 100.);
  ToTConverter conv(params);
  BOOST_CHECK(conv.getParams().fValidFunction);
  BOOST_CHECK_CLOSE(conv(0), getToT1(0), 0.1);
  BOOST_CHECK_CLOSE(conv(1), getToT1(1), 0.1);
  BOOST_CHECK_CLOSE(conv(10), getToT1(10), 0.1);
  BOOST_CHECK_CLOSE(conv(59.5), getToT1(59.5), 0.1);
  BOOST_CHECK_CLOSE(conv(99.9), getToT1(99.9), 0.1);
}

BOOST_AUTO_TEST_CASE(getTot_quadratic)
{
  ToTConverterParams params("pol2", {1, 1, 1}, 10000, 0., 100.); /// 1 + x + x^2
  ToTConverter conv(params);
  BOOST_CHECK(conv.getParams().fValidFunction);
  BOOST_CHECK_CLOSE(conv(0), 1, 0.1);
  BOOST_CHECK_CLOSE(conv(1), 3, 0.1);
  BOOST_CHECK_CLOSE(conv(2), 7, 0.1);
  BOOST_CHECK_CLOSE(conv(3), 13, 0.1);
}


BOOST_AUTO_TEST_SUITE_END()
