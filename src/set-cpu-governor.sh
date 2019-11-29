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

gov=$1

APP=""
APP_CPUPOWER="cpupower"
APP_CPUFREQ="cpufreq-set"

command -v $APP_CPUPOWER > /dev/null
if [ $? -eq 0 ]; then
	echo "Using $APP_CPUPOWER"
	APP="$APP_CPUPOWER frequency-set -g"
fi

command -v $APP_CPUFREQ > /dev/null
if [ $? -eq 0 ]; then
	echo "Using $APP_CPUFREQ"
	APP="$APP_CPUFREQ -r -g"
fi

if [ "$APP" = "" ]; then
	echo "Could not find $APP_CPUPOWER or $APP_CPUFREQ"
else
	echo "Setting all CPU governors to '$gov'"
	sudo $APP $gov
fi

