#include "InterfileMLPReconstruction.h"
#include "EventCategorizerTools.h"
#include <TH3D.h>
#include <TVector3.h>
#include "JPetOptionsTools/JPetOptionsTools.h"
#include "JPetEvent/JPetEvent.h"
using namespace jpet_options_tools;

InterfileMLPReconstruction::InterfileMLPReconstruction(const char *name) : JPetUserTask(name) {}

InterfileMLPReconstruction::~InterfileMLPReconstruction() {}

bool InterfileMLPReconstruction::init()
{
  setUpOptions();
  fOutputEvents = new JPetTimeWindow("JPetEvent");

  getStatistics().createHistogram(new TH3D("hits_pos",
                                           "Reconstructed hit pos",
                                           fNumberOfBinsX, -fXRange, fXRange,
                                           fNumberOfBinsY, -fYRange, fYRange,
                                           fNnumberOfBinsZ, -fZRange, fZRange));
  return true;
}

bool InterfileMLPReconstruction::exec()
{
  if (const auto &timeWindow = dynamic_cast<const JPetTimeWindow *const>(fEvent))
  {
    unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++)
    {
      auto event = dynamic_cast<const JPetEvent &>(timeWindow->operator[](static_cast<int>(i)));
      auto numberOfHits = event.getHits().size();
      if (numberOfHits <= 1)
        continue;
      else
      {
        auto hits = event.getHits();
        for (unsigned int i = 0; i < hits.size() - 1; i++)
        {
          TVector3 annihilationPoint = EventCategorizerTools::calculateAnnihilationPoint(hits[i], hits[i + 1]);
          getStatistics().getObject<TH3D>("hits_pos")->Fill(annihilationPoint.X(), annihilationPoint.Y(), annihilationPoint.Z());
        }
      }
    }
  }
  else
  {
    ERROR("Returned event is not TimeWindow");
    return false;
  }
  return true;
}

bool InterfileMLPReconstruction::terminate()
{
  std::ofstream fOutputStream;
  fOutputStream.open(fOutputPath + ".img", std::ios::binary | std::ios::out);

  auto pos_histo = getStatistics().getObject<TH3D>("hits_pos");

  auto x_bins = pos_histo->GetNbinsX();
  auto y_bins = pos_histo->GetNbinsY();
  auto z_bins = pos_histo->GetNbinsZ();

  //Intefile format is defined in a way, that we saves 2D arrays
  //from top left corner to bottom right
  //in row major order, more information in article:
  //https://www.researchgate.net/publication/21707960_A_file_format_for_the_exchange_of_nuclear_medicine_image_data_A_specification_of_interfile_version_33

  for (auto i = 0; i < z_bins; i++) {
    for (auto j = y_bins; j > 0; j--) {
      auto bin_pos = pos_histo->GetBin(1, j, i + 1);
      for (auto k = 0; k < x_bins; k++) {
        auto val = static_cast<float>(pos_histo->GetBinContent(bin_pos + k));
        fOutputStream.write(reinterpret_cast<char *>(&val), sizeof(val));
      }
    }
  }

  fOutputStream.close();

  fOutputStream.open(fOutputPath + ".hdr", std::ios::out);
  fOutputStream << "X size: " << x_bins << " " << std::endl
                << "Y size: " << y_bins << " " << std::endl
                << "Z size: " << z_bins << " " << std::endl
                << "type: float" << std::endl;
  fOutputStream.close();

  return true;
}

void InterfileMLPReconstruction::setUpOptions()
{
  auto opts = getOptions();

  if (isOptionSet(opts, kOutputPath))
  {
    fOutputPath = getOptionAsString(opts, kOutputPath);
  }

  if (isOptionSet(opts, kXRangeOn3DHistogramKey))
  {
    fXRange = getOptionAsInt(opts, kXRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kYRangeOn3DHistogramKey))
  {
    fYRange = getOptionAsInt(opts, kYRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kZRangeOn3DHistogramKey))
  {
    fZRange = getOptionAsInt(opts, kZRangeOn3DHistogramKey);
  }

  if (isOptionSet(opts, kBinMultiplierKey))
  { //sets-up bin size in root 3d-histogram, root cannot write more then 1073741822 bytes to 1 histogram
    const int kMaxRootFileSize = 1073741822;
    fBinMultiplier = getOptionAsDouble(opts, kBinMultiplierKey);
    if ((std::floor(fBinMultiplier * fXRange) *
         std::floor(fBinMultiplier * fYRange) *
         std::floor(fBinMultiplier * fZRange)) > kMaxRootFileSize)
    {
      fBinMultiplier = 6;
      WARNING("TBufferFile can only write up to 1073741822 bytes, bin multiplier is too big, reseted to 6");
    }
    fNumberOfBinsX = fXRange * fBinMultiplier;
    fNumberOfBinsY = fYRange * fBinMultiplier;
    fNnumberOfBinsZ = fZRange * fBinMultiplier;
  }
}
