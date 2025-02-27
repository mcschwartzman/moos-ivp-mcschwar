#!/bin/bash -e
#----------------------------------------------------------
#  Script: launch.sh
#  Author: Michael Benjamin
#  LastEd: May 20th 2019
#----------------------------------------------------------
#  Part 1: Set Exit actions and declare global var defaults
#----------------------------------------------------------
TIME_WARP=1
COMMUNITY="bravo"
GUI="yes"
DEPTH_THRESH=0
DISTANCE=200

#----------------------------------------------------------
#  Part 2: Check for and handle command-line arguments
#----------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	echo "launch.sh [SWITCHES] [time_warp]   "
	echo "  --help, -h           Show this help message            " 
	exit 0;
    elif [ "${ARGI}" = "--nogui" ] ; then
	GUI="no"
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [[ $ARGI =~ (--depth_thresh=)([^ ]*) ]]; then 
        DEPTH_THRESH="${BASH_REMATCH[2]}"
    elif [[ $ARGI =~ (--distance=)([^ ]*) ]]; then 
        DISTANCE="${BASH_REMATCH[2]}"
    else 
        echo "launch.sh Bad arg:" $ARGI " Exiting with code: 1"
        exit 1
    fi
done


#----------------------------------------------------------
#  Part 3: Launch the processes
#----------------------------------------------------------
echo "Launching $COMMUNITY MOOS Community with WARP:" $TIME_WARP
pAntler $COMMUNITY.moos --MOOSTimeWarp=$TIME_WARP >& /dev/null &

sleep 2

uPokeDB ODOMETRY_UPDATES="depth_threshold=$DEPTH_THRESH" bravo.moos++
uPokeDB RETURN_UPDATE="condition=ODOMETRY_DIST_AT_DEPTH>$DISTANCE" bravo.moos++

uMAC -t $COMMUNITY.moos
kill -- -$$
