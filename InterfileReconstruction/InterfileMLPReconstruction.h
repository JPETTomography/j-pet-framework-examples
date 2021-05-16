#ifndef INTERFILEMLPRECONSTRUCTION_H
#define INTERFILEMLPRECONSTRUCTION_H

#include "JPetUserTask/JPetUserTask.h"
#include <memory>

class InterfileMLPReconstruction : public JPetUserTask
{

public:
  InterfileMLPReconstruction(const char* name);
  virtual ~InterfileMLPReconstruction();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

private:
  void setUpOptions();

  const std::string kOutputPath             = "InterfileMLPReconstruction_OutputPath_std::string";
  const std::string kXRangeOn3DHistogramKey = "InterfileMLPReconstruction_Xrange_On_3D_Histogram_int";
  const std::string kYRangeOn3DHistogramKey = "InterfileMLPReconstruction_Yrange_On_3D_Histogram_int";
  const std::string kZRangeOn3DHistogramKey = "InterfileMLPReconstruction_Zrange_On_3D_Histogram_int";
  const std::string kBinMultiplierKey       = "InterfileMLPReconstruction_Bin_Multiplier_double";

  std::string fOutputPath = "out";
  int fXRange = 50;
  int fYRange = 50;
  int fZRange = 30;
  double fBinMultiplier = 12;
  int fNumberOfBinsX = fXRange * fBinMultiplier; //1 bin is 8.33 mm, TBufferFile cannot write more then 1073741822 bytes
  int fNumberOfBinsY = fYRange * fBinMultiplier;
  int fNnumberOfBinsZ = fZRange * fBinMultiplier;
};

#endif /*  !INTERFILEMLPRECONSTRUCTION_H */
