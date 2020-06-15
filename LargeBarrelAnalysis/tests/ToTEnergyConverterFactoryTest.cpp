#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ToTEnergyConverterFactoryTest

#include "../ToTEnergyConverterFactory.h"
#include "JPetLoggerInclude.h"
#include <boost/test/unit_test.hpp>
using namespace jpet_common_tools;
using namespace tot_energy_converter;

/// Returns Time-over-threshold for given deposited energy
/// the current parametrization is par1 + par2 * eDep
/// Returned value in ps, and eDep is given in keV.
double getToT1(double eDep, double par1 = -91958, double par2 = 19341)
{
  if (eDep < 0)
    return 0;
  double value = par1 + eDep * par2;
  return value;
}

BOOST_AUTO_TEST_SUITE(ToTEnergyConverterFactoryTestSuite)

BOOST_AUTO_TEST_CASE(test1)
{
  std::string formula1 = "pol1";
  std::vector<double> params1 = {-91958, 19341};
  std::vector<double> limits1 = {0, 100};

  std::string formula2 = "[0] + [1] * TMath::Log(x)";
  std::vector<double> params2 = {1, -2};
  std::vector<double> limits2 = {2, 100};

  std::map<std::string, boost::any> options = {{"ToTEnergyConverterFactory_Energy2ToTParameters_std::vector<double>", params1},
                                               {"ToTEnergyConverterFactory_Energy2ToTFunction_std::string", formula1},
                                               {"ToTEnergyConverterFactory_Energy2ToTFunctionLimits_std::vector<double>", limits1},
                                               {"ToTEnergyConverterFactory_ToT2EnergyParameters_std::vector<double>", params2},
                                               {"ToTEnergyConverterFactory_ToT2EnergyFunction_std::string", formula2},
                                               {"ToTEnergyConverterFactory_ToT2EnergyFunctionLimits_std::vector<double>", limits2}};

  ToTEnergyConverterFactory fact;
  fact.loadOptions(options);
  auto conv = fact.getToTConverter();
  BOOST_CHECK_CLOSE(conv(0), getToT1(0), 0.1);
  BOOST_CHECK_CLOSE(conv(1), getToT1(1), 0.1);
  BOOST_CHECK_CLOSE(conv(10), getToT1(10), 0.1);
  BOOST_CHECK_CLOSE(conv(59.5), getToT1(59.5), 0.1);
  BOOST_CHECK_CLOSE(conv(99.9), getToT1(99.9), 0.1);

  auto conv2 = fact.getEnergyConverter();

  TF1 funcTest("funcTest", "[0] + [1] * TMath::Log(x)", 2, 100);
  funcTest.SetParameters(1, -2);
  BOOST_CHECK_CLOSE(conv2(3), funcTest.Eval(3), 0.1);
  BOOST_CHECK_CLOSE(conv2(5.5), funcTest.Eval(5.5), 0.1);
  BOOST_CHECK_CLOSE(conv2(45.25), funcTest.Eval(45.25), 0.1);
  BOOST_CHECK_CLOSE(conv2(91), funcTest.Eval(91), 0.1);
}

BOOST_AUTO_TEST_SUITE_END()
