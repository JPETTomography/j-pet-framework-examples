#ifndef RECONSTRUCTOR_H
#define RECONSTRUCTOR_H

#include <TMath.h>
#include <TRotation.h>
#include <TVector3.h>

#include "Hits/JPetBaseHit/JPetBaseHit.h"

using namespace std;

class Reconstructor
{

public:
  static Reconstructor* GetInstance();

  void setGamma(ushort i, const JPetBaseHit* hit);

  void setPromptGamma(const JPetBaseHit* hit);

  int getSolution(TVector3& point, Double_t& time, int whichSolution = 1);

  double getPScreationTime();
  double getPSlifetime();

  void setBarrelLength(double length);
  void setStripDimensions(double width, double height);

  void fillParamsArray(Float_t* params);
  void fillShiftsArray(Float_t* shifts);

  // stuff for a kinematic fit with a cyllindrical source
  void setChamberRadius(double r) { fChamberRadius = r; }

  double fcn(const double* x);
  int kinematicFit(float& chisq);

  // Kamil's recs
  TVector3 RecoPos(const JPetBaseHit* Hit1, const JPetBaseHit* Hit2, const JPetBaseHit* Hit3);
  TVector3 FindIntersection2(TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double t21, double t31);
  double FindMinFromDerrivative(vector<double> Arg, vector<double> Val);
  vector<vector<double>> FindIntersectionPointsOfCircles(TVector3 Hit1Pos, TVector3 Hit2Pos, TVector3 Hit3Pos, double R1, double R2, double R3,
                                                         double R13, double R21, double R32);

  double CalcDistFromCentres(TVector3 Sol1, TVector3 Gamma1, TVector3 Gamma2, TVector3 Gamma3);
  double CalcDistAdd(const JPetBaseHit* Hit1);

private:
  int reconstruct(); // returs errFlag

  // stuff for the kinematic fit
  static Reconstructor* _this;

  TVector3 getGammaHit(int i) const;
  Double_t getGammaTime(int i) const;

  int n_fit_params;
  void setParameters(const double* x);
  double getChisq(const double* x) const;
  double p0[12];
  double sigmas[12];

  Double_t gammas_time_shift[3];
  TVector3 gammas_shift[3]; // shift w.r.t. original hit (centers in X,Y, recorded Z in Z)

  // end of stuff for the kinematic fit

  TVector3 prompt_gamma_hit;
  Double_t prompt_gamma_time;
  TVector3 gammas_hit[3];
  double gammas_polar_angle[3];
  Double_t gammas_time[3];
  TVector3 sol_hit[2];
  Double_t sol_time[2];

  static const Double_t cvel;

  // detector details
  double fStripWidth;
  double fStripHeight;
  double fStripLength;
  double fChamberRadius;
  double fChamberLength;
};

#endif // RECONSTRUCTOR_H
