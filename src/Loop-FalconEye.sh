#!/bin/bash
##
## Copyright (c) 2019 Robert Falkenberg.
##
## This file is part of FALCON 
## (see https://github.com/falkenber9/falcon).
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU Affero General Public License as
## published by the Free Software Foundation, either version 3 of the
## License, or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Affero General Public License for more details.
##
## A copy of the GNU Affero General Public License can be found in
## the LICENSE file in the top-level directory of this distribution
## and at http://www.gnu.org/licenses/.
##

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#BUILDDIR="$SCRIPTDIR/../build/src/examples"
LOGDIR="."

INTERVAL_SEC=1

APPLICATION="FalconEye"
THE_DATE=`date +"%Y-%m-%d-%H-%M-%S"`
HOSTNAME=`hostname`
APP_STDOUT="$LOGDIR/$THE_DATE-$HOSTNAME-Loop-FalconEye.log"

NOF_SUBFRAMES="300000"
FREQ="1815e6"

#OPERATOR="Telekom_"
#OPERATOR="Vodafone"
#OPERATOR="O2-DE___"
#OPERATOR="PCI165__"	#North
#OPERATOR="PCI166__"	#West
#OPERATOR="PCI167__"	#East
#CELL_PCI="-l 1"		# empty for 'best cell'
#CELL_PCI="-l 0"		# North
#CELL_PCI="-l 1"		# West
#CELL_PCI="-l 2"		# East

source "Settings-Loop-FalconEye.sh"

APP_CONST_PARAMS="-r -f $FREQ -n $NOF_SUBFRAMES $CELL_PCI"

echo  $SCRIPTDIR
echo  $APPLICATION
echo  $APP_CONST_PARAMS

while true;
do
	THE_DATE=`date +"%Y-%m-%d-%H-%M-%S"`
	BASENAME="$THE_DATE-$OPERATOR-falcon"
	APP_STDOUT="$LOGDIR/$BASENAME.log"
	
	echo "Starting Measurement $BASENAME" | tee -a $APP_STDOUT
	echo ./$APPLICATION $APP_CONST_PARAMS -D "$BASENAME-dci.csv" -E "$BASENAME-stats.csv" | tee -a $APP_STDOUT
	./$APPLICATION $APP_CONST_PARAMS -D "$BASENAME-dci.csv" -E "$BASENAME-stats.csv" 2>&1 | tee -a $APP_STDOUT
	echo "Pause for $INTERVAL_SEC s" | tee -a $APP_STDOUT
	sleep $INTERVAL_SEC
done;
