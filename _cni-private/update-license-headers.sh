#!/bin/bash

directories=(
  "../lib"
  "../src"
)

LICENSE="agpl-v3-falcon"
YEARS="2019"
OWNER="Robert Falkenberg"
PROGRAM_NAME="FALCON"
PROGRAM_URL="https://github.com/falkenber9/falcon"

for target in ${directories[*]}; do
  echo "Processing directory $target"
  licenseheaders/licenseheaders.py -t $LICENSE -y "$YEARS" -o "$OWNER" -n "$PROGRAM_NAME" -u "$PROGRAM_URL" -d $target
done
