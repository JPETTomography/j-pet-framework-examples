/**
 *  @copyright Copyright 2018 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file MLEMRunner.cpp
 */

#include "JPetGeomMapping/JPetGeomMapping.h"
#include "2d/gate/gate_scanner_builder.h"
#include "2d/gate/gate_volume_builder.h"
#include "JPetParamBank/JPetParamBank.h"
#include "JPetHit/JPetHit.h"
#include "MLEMRunner.h"
#include <iomanip> //std::setprecision

using namespace jpet_options_tools;

MLEMRunner::MLEMRunner(const char* name) : JPetUserTask(name)
{
  Gate::D2::Volume<F>* world = Gate::D2::build_big_barrel_volume<F>();
  auto builder = new Gate::D2::GenericScannerBuilder<F, S, 512>;
  fSymmetryDescriptor = builder->build_with_8_symmetries(world);
  delete world;
}

MLEMRunner::~MLEMRunner()
{
  fOutputStream.close();
  delete fMatrix;
  delete fReconstruction;
  delete fGrid2d;
  delete fGrid;
  delete fScannerReconstruction;
  delete fGeometrySOA;
}

bool MLEMRunner::init()
{
  setUpOptions();
  return setUpRunReconstructionWithMatrix();
}

bool MLEMRunner::exec()
{
  if (const auto& timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    const unsigned int numberOfEventsInTimeWindow = timeWindow->getNumberOfEvents();
    for (unsigned int i = 0; i < numberOfEventsInTimeWindow; i++) {
      const auto event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](static_cast<int>(i)));
      if (!parseEvent(event))
        fMissedEvents++;
      else
        fReconstructedEvents++;
    }
  }
  return true;
}

bool MLEMRunner::terminate()
{
  INFO("Reconstructed " + std::to_string(fReconstructedEvents) + " events out of " + std::to_string(fReconstructedEvents + fMissedEvents) +
       " total(" + std::to_string(fMissedEvents) + " missed)");
  runReconstruction();
  return true;
}

bool MLEMRunner::parseEvent(const JPetEvent& event)
{
  const auto hits = event.getHits();
  if (hits.size() != 2) {
    return false;
  }
  const float x1 = hits[0].getPosX();
  const float y1 = hits[0].getPosY();
  float z1 = hits[0].getPosZ();
  float t1 = hits[0].getTime();

  const float x2 = hits[1].getPosX();
  const float y2 = hits[1].getPosY();
  float z2 = hits[1].getPosZ();
  float t2 = hits[1].getTime();

  if (std::abs(z1 * kCentimetersToMeters) > fHalfStripLenght || std::abs(z2 * kCentimetersToMeters) > fHalfStripLenght) {
    return false;
  }

  int d1 = -1, d2 = -1;
  for (size_t i = 0; i < fScanner.size(); ++i) {
    if (fScanner[i].contains(Point(x1 * kCentimetersToMeters, y1 * kCentimetersToMeters), EPSILON))
      d1 = i;
    if (fScanner[i].contains(Point(x2 * kCentimetersToMeters, y2 * kCentimetersToMeters), EPSILON))
      d2 = i;
  }

  if (d1 < 0 || d2 < 0) {
    return false;
  }

  if (d1 < d2) {
    std::swap(d1, d2);
    std::swap(z1, z2);
    std::swap(t1, t2);
  }
  double dl = (t1 - t2) * kSpeedOfLightMetersPerPs;

  fReconstructionStringStream << d1 << " " << d2 << " " << z1* kCentimetersToMeters << " " << z2* kCentimetersToMeters << " " << dl << "\n";
  fOutputStream << d1 << " " << d2 << " " << z1* kCentimetersToMeters << " " << z2* kCentimetersToMeters << " " << dl << "\n";

  return true;
}

void MLEMRunner::setUpOptions()
{
  auto opts = getOptions();
  if (isOptionSet(opts, kOutFileNameKey)) {
    fOutFileName = getOptionAsString(opts, kOutFileNameKey);
  }

  fOutputStream.open(fOutFileName, std::ofstream::out | std::ofstream::app);
  std::vector<float> radius;
  radius.reserve(3);
  std::vector<int> scintillators;
  scintillators.reserve(3);

  const std::vector<float> rotation{0.f, 0.5f, 0.5f}; // rotation for BigBarrel

  const JPetParamBank& bank = getParamBank();
  const JPetGeomMapping mapping(bank);
  for (unsigned int i = 1; i < mapping.getLayersCount(); i++) {
    radius.push_back(mapping.getRadiusOfLayer(i) * kCentimetersToMeters);
    scintillators.push_back(static_cast<int>(mapping.getSlotsCount(i)));
  }

  const auto scin = bank.getScintillator(1);

  const float detectorWidth = scin.getScinSize(JPetScin::Dimension::kWidth);
  const float detectorHeight = scin.getScinSize(JPetScin::Dimension::kHeight);
  fHalfStripLenght = scin.getScinSize(JPetScin::Dimension::kLength) / 2.f;
  const float detectorD = 0.f;
  const float fowRadius = 0.4f;

  fScanner = ScannerBuilder<SquareScanner>::build_multiple_rings(__PET2D_BARREL(radius,         // radius
             rotation,       // rotation
             scintillators,  // n-detectors
             detectorWidth,  // w-detector
             detectorHeight, // h-detector
             detectorD,      // should be d-detector
             fowRadius       // fow radius
                                                                               ));

  if (isOptionSet(opts, kNumberOfPixelsInOneDimensionKey)) {
    fNumberOfPixelsInOneDimension = getOptionAsInt(opts, kNumberOfPixelsInOneDimensionKey);
  }

  if (isOptionSet(opts, kPixelSizeKey)) {
    fPixelSize = getOptionAsDouble(opts, kPixelSizeKey);
  }

  if (isOptionSet(opts, kStartPixelForPartialMatrixKey)) {
    fStartPixelForPartialMatrix = static_cast<unsigned int>(getOptionAsInt(opts, kStartPixelForPartialMatrixKey));
  }

  if (isOptionSet(opts, kNumberOfEmissionsPerPixelKey)) {
    fNumberOfEmissionsPerPixel = static_cast<unsigned int>(getOptionAsInt(opts, kNumberOfEmissionsPerPixelKey));
  }

  if (isOptionSet(opts, kTOFStepKey)) {
    fTOFStep = getOptionAsDouble(opts, kTOFStepKey);
  }

  if (isOptionSet(opts, kVerboseKey)) {
    fVerbose = getOptionAsBool(opts, kVerboseKey);
  }

  if (isOptionSet(opts, kSystemMatrixOutputPathKey)) {
    fSystemMatrixOutputPath = getOptionAsString(opts, kSystemMatrixOutputPathKey);
  }

  if (isOptionSet(opts, kSystemMatrixSaveFullKey)) {
    fSystemMatrixSaveFull = getOptionAsBool(opts, kSystemMatrixSaveFullKey);
  }

  if (isOptionSet(opts, kReconstructionOutputPathKey)) {
    fReconstructionOutputPath = getOptionAsString(opts, kReconstructionOutputPathKey);
  }

  if (isOptionSet(opts, kReconstructionIterationsKey)) {
    fReconstructionIterations = getOptionAsInt(opts, kReconstructionIterationsKey);
  }
}

void MLEMRunner::setSystemMatrix()
{
  util::ibstream matrixStream(fSystemMatrixOutputPath);
  if (matrixStream.good()) {
    fMatrix = new SquareMatrix(matrixStream);
    if (fMatrix->n_emissions() != static_cast<int>(fNumberOfEmissionsPerPixel)) { // check is readed system matrix have desire number of emissions
      delete fMatrix;
      fMatrix = new SquareMatrix(runGenerateSystemMatrix()); // if not rerun generation of system matrix
    }
  } else {
    fMatrix = new SquareMatrix(runGenerateSystemMatrix());
  }

  if (fMatrix->triangular()) {
    fMatrix = new SquareMatrix(fMatrix->to_full(fSymmetryDescriptor.symmetry_descriptor()));
  }
}

bool MLEMRunner::setUpRunReconstructionWithMatrix()
{
  setSystemMatrix();

  int n_planes = -1;
  double z_left = 0.;

  std::cout << "Assumed: " << std::endl;
  if (n_planes == -1) {
    n_planes = fNumberOfPixelsInOneDimension;
    std::cout << "--n-planes=" << n_planes << std::endl;
  }
  if (z_left == 0.) {
    z_left = -n_planes * fPixelSize / 2;
    std::cout << "--z-left=" << z_left << std::endl;
  }

  float scannerLenght = 2.f;
  float fTOFsigmaAlongZAxis = 0.015f;
  float fTOFSigmaAxis = 0.06f;

  fScannerReconstruction = new ScannerReconstruction(fSymmetryDescriptor, scannerLenght);
  fScannerReconstruction->set_sigmas(fTOFsigmaAlongZAxis, fTOFSigmaAxis);

  if (fMatrix->n_detectors() != (int)fScannerReconstruction->barrel.size()) {
    ERROR("Number of detectors in system matrix and reconstruction scanner mismatch, returning from reconstruction");
    return false;
  }

  fGrid2d = new Geometry::Grid(fMatrix->n_pixels_in_row(), fMatrix->n_pixels_in_row(), fPixelSize);

  if (fVerbose) {
    std::cout << "3D hybrid reconstruction w/system matrix:" << std::endl << "    detectors = " << fMatrix->n_detectors() << std::endl;
    std::cerr << "   pixel grid = " // grid size:
              << fGrid2d->n_columns << " x " << fGrid2d->n_rows << " / " << fGrid2d->pixel_size << std::endl;
  }

  fGrid = new Reconstruction::Grid(*fGrid2d, z_left, n_planes);

  fGeometrySOA = new Reconstruction::Geometry(*fMatrix, fScannerReconstruction->barrel.detector_centers(), (*fGrid2d));
  if (fVerbose) {
    std::cerr << "system matrix = " << fSystemMatrixOutputPath << std::endl;
  }

  fReconstruction = new Reconstruction((*fScannerReconstruction), (*fGrid), (*fGeometrySOA), false);

  if (fVerbose) {
    std::cerr << "   voxel grid = " // grid size:
              << fReconstruction->grid.pixel_grid.n_columns << " x " << fReconstruction->grid.pixel_grid.n_columns << " x "
              << fReconstruction->grid.n_planes << std::endl;
  }

  fReconstruction->calculate_sensitivity();
  fReconstruction->normalize_geometry_weights();
  return true;
}

MLEMRunner::SquareMatrix MLEMRunner::runGenerateSystemMatrix()
{
  Common::ScintillatorAccept<F> model(0.1f);

  int n_tof_positions = 1;
  double max_bias = 0;
  if (fTOFStep > 0.) {
    n_tof_positions = fSymmetryDescriptor.n_tof_positions(fTOFStep, max_bias);
  }

  std::random_device rd;
  util::random::tausworthe rng(rd());
  if (fVerbose) {
    std::cerr << "Monte-Carlo:" << std::endl;
  }

#if _OPENMP
  std::cerr << " OpenMP threads = " << omp_get_max_threads() << std::endl;
#endif

  std::cerr << "  pixels in row = " << fNumberOfPixelsInOneDimension << std::endl;
  std::cerr << "     pixel size = " << fPixelSize << std::endl;
  std::cerr << "     fov radius = " << fSymmetryDescriptor.fov_radius() << std::endl;
  std::cerr << "   outer radius = " << fSymmetryDescriptor.outer_radius() << std::endl;
  std::cerr << "       max bias = " << max_bias << std::endl;
  std::cerr << "       TOF step = " << fTOFStep << std::endl;
  std::cerr << "  TOF positions = " << n_tof_positions << std::endl;
  std::cerr << "      emissions = " << fNumberOfEmissionsPerPixel << std::endl;

  ComputeMatrix::SparseMatrix sparse_matrix(fNumberOfPixelsInOneDimension, fSymmetryDescriptor.size(), n_tof_positions);

  ComputeMatrix matrix(fNumberOfPixelsInOneDimension, fSymmetryDescriptor.size(), n_tof_positions);
  util::progress progress(fVerbose, matrix.total_n_pixels_in_triangle, 1);

  if (fNumberOfEmissionsPerPixel > 0) {
    if (!sparse_matrix.empty()) {
      matrix << sparse_matrix;
      sparse_matrix.resize(0);
    }

    PET2D::Barrel::MonteCarlo<Scanner2D, ComputeMatrix> monte_carlo(fSymmetryDescriptor, matrix, fPixelSize, fTOFStep, fStartPixelForPartialMatrix);
    monte_carlo(rng, model, fNumberOfEmissionsPerPixel, progress);
    sparse_matrix = matrix.to_sparse();
  }

  util::obstream out(fSystemMatrixOutputPath, std::ios::binary | std::ios::trunc);

  if (fSystemMatrixSaveFull) {
    auto full_matrix = sparse_matrix.to_full(fSymmetryDescriptor.symmetry_descriptor());
    out << full_matrix;
  } else {
    out << sparse_matrix;
  }

  try {
    util::png_writer png(fSystemMatrixOutputPath + ".png");
    sparse_matrix.output_bitmap(png);
  } catch (const char* ex) {
    // don't bail out just produce warning
    WARNING("Exception when creating output png file: " + std::string(ex));
  }

  util::svg_ostream<F> svg(fSystemMatrixOutputPath + ".svg", fSymmetryDescriptor.outer_radius(), fSymmetryDescriptor.outer_radius(), 1024., 1024.);
  svg.link_image(fSystemMatrixOutputPath + ".png", -(fPixelSize * fNumberOfPixelsInOneDimension) / 2,
                 -(fPixelSize * fNumberOfPixelsInOneDimension) / 2, fPixelSize * fNumberOfPixelsInOneDimension,
                 fPixelSize * fNumberOfPixelsInOneDimension);

  svg << fSymmetryDescriptor;

  return sparse_matrix.to_full(fSymmetryDescriptor.symmetry_descriptor());
}

void MLEMRunner::runReconstruction()
{
  (*fReconstruction) << fReconstructionStringStream;
  if (fVerbose) {
    Reconstruction::EventStatistics st;
    fReconstruction->event_statistics(st);
    std::cerr << "       events = " << fReconstruction->n_events() << std::endl;
    std::cerr
    // event pixels ranges:
        << "  pixels: "
        << "min = " << st.min_pixels << ", "
        << "max = " << st.max_pixels << ", "
        << "avg = " << st.avg_pixels
        << std::endl
        // event planes ranges:
        << "  planes: "
        << "min = " << st.min_planes << ", "
        << "max = " << st.max_planes << ", "
        << "avg = " << st.avg_planes
        << std::endl
        // event voxels ranges:
        << "  voxels: "
        << "min = " << st.min_voxels << ", "
        << "max = " << st.max_voxels << ", "
        << "avg = " << st.avg_voxels << std::endl;
  }

  auto n_blocks = fReconstructionIterations;
  int n_iterations_in_block = 1;
  auto n_iterations = fReconstructionIterations;
  int start_iteration = 0;
  util::progress progress(fVerbose, n_iterations, 1, start_iteration);

  for (int block = 0; block < n_blocks; ++block) {
    for (int i = 0; i < n_iterations_in_block; i++) {
      const auto iteration = block * n_iterations_in_block + i;
      if (iteration < start_iteration)
        continue;
      progress(iteration);
      (*fReconstruction)();
      progress(iteration, true);
    }
    const auto iteration = (block + 1) * n_iterations_in_block;
    if (iteration <= start_iteration)
      continue;
    util::nrrd_writer nrrd(fReconstructionOutputPath + "_" + std::to_string(iteration) + ".nrrd",
                           fReconstructionOutputPath + "_" + std::to_string(iteration), false);
    nrrd << fReconstruction->rho;
    util::obstream bin(fReconstructionOutputPath + "_" + std::to_string(iteration));
    bin << fReconstruction->rho;
  }
  // final reconstruction statistics
  const auto st = fReconstruction->statistics();
  std::cerr << "  event count = " << st.used_events << std::endl;
  std::cerr << "  voxel count = " << st.used_voxels << "(" << (double)st.used_voxels / st.used_events << " / event)" << std::endl;
  std::cerr << "  pixel count = " << st.used_pixels << "(" << (double)st.used_pixels / st.used_events << " / event)" << std::endl;
}
