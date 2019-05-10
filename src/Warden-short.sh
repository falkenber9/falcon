#!/bin/bash

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
