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

## Parameters controlling Z position smearing:

- "GeantParser_ZPositionSmearingParameters_std::vector<double>"
Parameters passed to the Z position smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GeantParser_ZPositionSmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared Z position.
Arguments: `double* x` - variable smeared Z position [cm]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

- "GeantParser_ZPositionSmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in cm) around the MC-generated Z position value inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!

## Parameters controlling energy deposition smearing:

- "GeantParser_EnergySmearingParameters_std::vector<double>"
Parameters passed to the energy deposition smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GeantParser_EnergySmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared deposited energy value.
Arguments: `double* x` - variable smeared energy deposition [keV]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

-"GeantParser_EnergySmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in keV) around the MC-generated Edep value inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!

## Parameters controlling hit time smearing:

- "GeantParser_TimeSmearingParameters_std::vector<double>"
Parameters passed to the hit time smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GeantParser_TimeSmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared hit time.
Arguments: `double* x` - variable smeared hit time [ps]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

- "GeantParser_TimeSmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in ps) around the MC-generated hit time inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!
