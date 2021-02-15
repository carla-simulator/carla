#!/bin/bash
#python "$SUMO_HOME/tools/ptlines2flows.py" -n osm.net.xml -e 3600 -p 600 --random-begin --seed 42 --ptstops osm_stops.add.xml --ptlines osm_ptlines.xml -o osm_pt.rou.xml --ignore-errors --vtype-prefix pt_ --verbose --flow-attributes 'departPos="0"'

export CMD="python $SUMO_HOME/tools/randomTrips.py"
export COMMON_OPTS="-n osm.net.xml --seed 42 -e 1800 --fringe-factor 1000 --fringe-threshold 0 --min-distance 300 --validate"
export TRIPATTRS='speedDev="0.1" departLane="best" departSpeed="max" departPos="last" sigma="0.8"'
export TRIPATTRS_MW='speedDev="0.1" departLane="free" departSpeed="max" departPos="last" sigma="0.8"'

# background traffic 
$CMD $COMMON_OPTS -p  1 -L --speed-exponent 3  -r osm.passenger.rou.xml -o osm.passenger.trips.xml --vehicle-class passenger --vclass passenger --prefix veh   --trip-attributes "$TRIPATTRS" --weights-output-prefix weights 
$CMD $COMMON_OPTS -p 10 -L --speed-exponent 3  -r osm.truck.rou.xml     -o osm.truck.trips.xml     --vehicle-class truck     --vclass truck     --prefix truck --trip-attributes "$TRIPATTRS"

# motorway only (needs more traffic), achieved by high speed exponent
export M="_mw"
$CMD $COMMON_OPTS -p 0.7 -L --speed-exponent 100  -r osm.passenger$M.rou.xml -o osm.passenger$M.trips.xml --vehicle-class passenger --vclass passenger --prefix veh$M   --trip-attributes "$TRIPATTRS_MW" --weights-output-prefix weights_mw
$CMD $COMMON_OPTS -p 4.0  -L --speed-exponent 100  -r osm.truck$M.rou.xml     -o osm.truck$M.trips.xml     --vehicle-class truck     --vclass truck     --prefix truck$M --trip-attributes "$TRIPATTRS_MW"

# additional motorway traffic 
export M="_mwb"
$CMD $COMMON_OPTS --begin 100 -p 1.6 -L --speed-exponent 100  -r osm.passenger$M.rou.xml -o osm.passenger$M.trips.xml --vehicle-class passenger --vclass passenger --prefix veh$M   --trip-attributes "$TRIPATTRS_MW" --weights-output-prefix weights_mw
$CMD $COMMON_OPTS --begin 100 -p 3.0  -L --speed-exponent 100  -r osm.truck$M.rou.xml     -o osm.truck$M.trips.xml     --vehicle-class truck     --vclass truck     --prefix truck$M --trip-attributes "$TRIPATTRS_MW"
