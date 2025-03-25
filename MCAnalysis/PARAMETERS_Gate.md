# MC Gate Analysis parameters description

- "GateParser_MakeHistograms_bool"
default value : true
Create standard analysis histograms

- "GateParser_EnergyThreshold_double"
default value [keV] : 10.0
Energy threshold applied by the electronics - different for different runs, so be careful while
applying. This value influence on number of reconstructed hits and events.

- "GateParser_ClockWindowTimeInps_double"
default value [ps] : 20000000.0
Electronics time window. Default value is for Modular J-PET

## Parameters controlling Z position smearing:

- "GateParser_ZPositionSmearingParameters_std::vector<double>"
Parameters passed to the Z position smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GateParser_ZPositionSmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared Z position.
Arguments: `double* x` - variable smeared Z position [cm]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

- "GateParser_ZPositionSmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in cm) around the MC-generated Z position value inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!

## Parameters controlling energy deposition smearing:

- "GateParser_EnergySmearingParameters_std::vector<double>"
Parameters passed to the energy deposition smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GateParser_EnergySmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared deposited energy value.
Arguments: `double* x` - variable smeared energy deposition [keV]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

- "GateParser_EnergySmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in keV) around the MC-generated Edep value inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!

## Parameters controlling hit time smearing:

- "GateParser_TimeSmearingParameters_std::vector<double>"
Parameters passed to the hit time smearing function specified using the next parameter.
This vector can have any number of elements, which will be available inside the smearing function as p[4], p[5] etc. (p[0]-p[3] are reserved for default parameters: (ScinID, Z, E, T)).

- "GateParser_TimeSmearingFunction_std::string"
String specifying a valid C++ lambda function which returns probability density function of the smeared hit time.
Arguments: `double* x` - variable smeared hit time [ps]
Parameters: `double* p` - an array of default parameters + user-defined parameters provided as described above (p[0]=ScinID, p[1]=Z, p[2]=E, p[3]=T, p[4]=user parameter 1, p[5]=user parameter 2, ...).

- "GateParser_TimeSmearingFunctionLimits_std::vector<double>"
Vector of two values specifying the limits (in ps) around the MC-generated hit time inside which the smearing should be allowed. Use limits large enough not to truncate your smearing probability distributions!
