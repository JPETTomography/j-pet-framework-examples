#!/bin/bash

#example for RUN1

./CalibrationMS.x -t hld -f LayerX_SlotY.hld -i 44 -u userParams_ref.json -p conf_trb3.xml -c TOT_runX.root

#userParams_ref.json -> add the following parameter to user options "TimeWindowCreator_MainStrip":"XXX", ex. "TimeWindowCreator_MainStrip":"243" -> Layer 2, Slot 43
#TOT_runX.root -> TOT calibration file for proper run
 

