# MCGateAnalysis

## Aim
This example demonstrates how to:
- load a simulation files created with GATE
- apply and control smearing of MC-generated values of hit Z position and hit time using user-provided parameters
- deposited energy is usually already smeared by GATE however the utility of smearing is available
- distinguish data and MC in an analysis module and access generated MC information when processing MC files

## Input and Output
- Input file must have the `*.mcGate.root`  
- Also required is a `.json` file with detector setup which corresponds to simulated/analyzed run.
Those files are the same as used while processing the data and can be taken from CalibrationFiles/X_RUN/detectorSetupRun.json
- Output files created   `*.hits.root`, `*.unk.evt.root`, `*.ana.evt.root`
- No output to stdout,`JPet_(date).log` file appears with the log of the processing and ROOT files.

## How to use?
Edit `userParams_Gate.json` to adjust the MC smearing functions and their parameters to match the simulated run conditions.

Then start processing with:
`./run_Gate.sh *.mcGate.root setup_description_json run_number`

Finally, inspect the produced files, e.g. the resolution histograms in the `ana.evt.root` file.

## Creating analysis modules to work on both MC and data
The `EventAnalyzer` module contained in this example shows how the users can create modules which can transparently work on both MC and data files, executing some additional actions in case of processing MC. For example, `EventAnalyzer` compares the exact simulated values of hit Z position and deposited energy with their smeared counterparts, filling resolution histograms which are useful for checking whether the user-provided smearing functions work as desired.

Please also note that in this example, `EventAnalyzer` is applied to the results of the standard `EventFinder` module from the `LargeBarrelAnalysis` example, which is completely agnostic of whether it is working on MC or data files.

## Controlling the MC smearing
When the `mcGate.root` MC files are processed, exact simulated values of:

- hit Z position

- hit time

are smeared to reflect the finite experimental resolutions.

Users can control this smearing by specifying the probability density distributions of the smeared quantities as user parameters in the `userParams_Gate.json` file. Please refer to the [PARAMETERS](PARAMETERS_Gate.md) file for details.

Each smearing PDF can depend on the following hit properties which are passed automatically as parameters:

- scintillator ID (`p[0]`)

- hit Z position (`p[1]`)

- hit deposited energy (`p[2]`)

- hit time(`p[3]`)

as well as any number of arbitrary parameters which are passed as `p[4]`, `p[5]` and so on.
