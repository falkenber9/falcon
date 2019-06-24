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

APPLICATION="FalconCaptureProbe"
THE_DATE=`date +"%Y-%m-%d-%H-%M-%S"`
HOSTNAME=`hostname`
APP_STDOUT="$LOGDIR/$THE_DATE-$HOSTNAME-Loop-FalconCaptureProbe.log"

BACKOFF=0
if [ "$HOSTNAME" = "cni-lte5" ]; then
	BACKOFF=0
elif [ "$HOSTNAME" = "cni-lte6" ]; then
	BACKOFF=1
elif [ "$HOSTNAME" = "cni-lte9" ]; then
	BACKOFF=2
else
	echo "Unknown hostname in lookup for backoff. Backoff set to $BACKOFF"
fi

APP_PARAMS="-c -b $BACKOFF -N 1"

echo  $SCRIPTDIR
#echo  $BUILDDIR
#echo  $LOGDIR
echo  $APPLICATION
echo  $APP_PARAMS

while true;
do
	THE_DATE=`date +"%Y-%m-%d-%H-%M-%S"`
	echo "$THE_DATE-$HOSTNAME" | tee -a $APP_STDOUT
	./$APPLICATION $APP_PARAMS 2>&1 | tee -a $APP_STDOUT
	echo "Pause for $INTERVAL_SEC s" | tee -a $APP_STDOUT
	sleep $INTERVAL_SEC
done;
