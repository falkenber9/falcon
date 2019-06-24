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

# ./FalconCaptureWarden -h
#Usage: ./FalconCaptureWarden [aghijnopswW] 
#        -a Broadcast IP address [default: 169.254.255.255]
#        -g NMEA GPS device path [default: use GPS from aux modem]
#        -h Show this help message
#        -i Poll interval in s [default: 1]
#        -j Auto mode interval in s [default: 60]
#        -n Number of subframes in ms [default: 20000], 0=unlimited
#        -o Probing delay/offset in ms [default: 10000]
#        -p Broadcast port [default: 4567]
#        -s Payload size in byte [default: 5242880]
#        -w Probing URL for upload [default: http://129.217.211.19:6137/index.html]
#        -W Probing URL for download [default: http://129.217.211.19:6137/testfiles/100MB.bin]
#        -T TX power sampling interval in us [default: 250000], 0=disabled


#auto mode interval (-j): ( probing_delay(5s) + probing_timeout(10s) ) * operators(3) = 45
# probin_timeout of 10s cancels file transfer of 5MB with datarate < 4Mbit/s

APPLICATION="./FalconCaptureWarden"
PARAMS="-j 50 -n 5000 -o 5000"

echo "Launching $APPLICATION $PARAMS"
$APPLICATION $PARAMS
