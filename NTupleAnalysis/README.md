# NTuple Analysis

Module for reading the files written with NTupler. Usage example:
./NTupleAnalysis.x -t ntu -i 38 -l setup.json -u userParams.jso -f file01.ntu.root

Where -t ntu is the flag for reading this format of file. Can be used with -b or -d options.
No user params needed, but a json file should be provided, an empty one (with {} inside).

Output: JPetTimeWindow with JPetEvents, that contain JPetBaseHits.
