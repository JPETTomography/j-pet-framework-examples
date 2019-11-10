# MC Geant Analysis parameters description

- "GeantParser_MaxTimeWindow_double"
default value [ps]: 0.0
Maximum value of the time window provided by the electronics. For most runs DAQ time windows are between -50 microseconds and 0. 

- "GeantParser_MinTimeWindow_double"
default value [ps]: -50000000.0
See comment above

- "GeantParser_SourceActivity_double"
default value [MBq]: 4.7
Activity for the analyzed run - see proper report at PetWiki

- "GeantParser_MakeHistograms_bool"
default value : true
Create standard analysis histograms

- "GeantParser_MakeEfficiencies_bool"
default value : false
Create additional set of histograms connected with analysis efficiencies

- "GeantParser_EnergyThreshold_double"
default value [keV] : 10.0
Energy threshold applied by the electronics - different for different runs, so be careful while
applying. This value influence on number of reconstructed hits and events.

- "GeantParser_ProcessSingleEventInWindow_bool"
default value : false
For testing purposes user can ask for processing single event per time window

Note that presently there are no parameters allowing to control the smearing of generated hit properties such as interaction time and Z position resolution. In order to tune them, please modify the corresponding functions in the `JPetSmearingFunctions` class directly in the `j-pet-framework` code. User parameters allowing to control the smearing will be added in the future versions.