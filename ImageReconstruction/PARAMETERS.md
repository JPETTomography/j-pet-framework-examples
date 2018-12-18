# ImageReconstruction parameters description

## Contents

Description of available parameters in ImageReconstruction example.

- `FilterEvents_Cut_On_Z_Value_float`  
  Events with Z value greater then this value will not be included in output file. [cm]

- `FilterEvents_Cut_On_LOR_Distance_From_Center_float`
  Events with LOR distance greater then this value will not be included in output file [cm]

- `FilterEvents_TOT_Min_Value_In_Ns_float`
  Events with sum of TOT with value less then this value will not be included in output file [ns]

- `FilterEvents_TOT_Max_Value_In_Ns_float`
  Events with sum of TOT with value greater then this value will not be included in output file [ns]

- `FilterEvents_Angle_Delta_Min_Value_float`
  Events that hits have angle differences value less then this value will not be included in output file [degrees]

- `MLEMRunner_OutFileName_std::string`
  Path to file where will be saved converted data for futher reconstruction in j-pet-mlem [string]

- `MLEMRunner_NumberOfPixelsInOneDimension_int`
  Number of pixels in reconstructed image in one demension [px]

- `MLEMRunner_PixelSize_double`
  Size of 1 pixel in reconstructed image [m]

- `MLEMRunner_StartPixelForPartialMatrix_int`
  Start pixel of partial matrix.

- `MLEMRunner_NumberOfEmissionsPerPixel_int`
  Number of emmisions per pixel for system matrix.

- `MLEMRunner_TOFStepSize_double`
  TOF step size [m]

- `MLEMRunner_TOFSigmaAxis_float`
  TOF sigma [m]

- `MLEMRunner_TOFSigmaAlongZAxis_float`
  TOF sigma along Z axis [m]

- `MLEMRunner_DisplayInfo_bool`
  If true display extra info about reconstruction into cerr

- `MLEMRunner_SystemMatrixOutputPath_std::string`
  Path to file where system matrix will be saved and from what file read system matrix if already generated

- `MLEMRunner_SystemMatrixSaveFull_bool`
  If true generate full system matrix, not faster 1/8 of the system matrix and using symetries reconstruct full matrix

- `MLEMRunner_ReconstructionOutputPath_std::string`
  Path to file where reconstruction will be saved

- `MLEMRunner_ReconstuctionIterations_int`
  Number of MLEM iterations

- `ImageReco_Annihilation_Point_Z_float`
  Maximum value of Z of reconstructed annihilation point to be included in reconstruction

- `ImageReco_Xrange_On_3D_Histogram_int`
  Range of X axis on 3D Histogram

- `ImageReco_Yrange_On_3D_Histogram_int`
  Range of Y axis on 3D Histogram

- `ImageReco_Zrange_On_3D_Histogram_int`
  Range of Z axis on 3D Histogram

- `ImageReco_Bin_Multiplier_double`
  Used to decrease size of bin, if bin multiplier is 1: 1 bin correspondes to 1 cm.

- `SinogramCreator_OutFileName_std::string`
  Path to file where sinogram will be saved.

- `SinogramCreator_ReconstructionDistanceAccuracy_float`
  Used to decrease size of bin, if reconstruction distance accuracy is 1: 1 bin correspondes to 1cm

- `SinogramCreator_SinogramZSplitNumber_int`
  Number of sinograms along Z axis. Z axis is divided by this number and then evenly split.

- `SinogramCreator_ScintillatorLenght_float`
  Lenght of the scintillator. [cm]

- `SinogramCreatorMC_OutFileName_std::string`
  Path to file where sinogram will be saved.

- `SinogramCreatorMC_ReconstructionDistanceAccuracy_float`
  Used to decrease size of bin, if reconstruction distance accuracy is 1: 1 bin correspondes to 1cm

- `SinogramCreatorMC_SinogramZSplitNumber_int`
  Number of sinograms along Z axis. Z axis is divided by this number and then evenly split.

- `SinogramCreatorMC_ScintillatorLenght_float`
  Lenght of the scintillator. [cm]

- `SinogramCreatorMC_MaxReconstructionRadius_float`
  Maximal possible reconstruction radius. [cm]

- `SinogramCreatorMC_InputDataPath_std::string`
  Path to file where input data is stored.
