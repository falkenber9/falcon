#!/bin/bash 

templatedir="../testdata"
builddir=$1
workdir=$2

uut="imdeaowl/imdea_cc_decoder"
uut_params="-i $templatedir/template-iq.bin -p 50 -P 2 -c 392 -n 1000 -D $workdir/test-owl-dci.csv -E $workdir/test-owl-stats.csv -d"

#exit immediately if any command fails
set -e

echo "Testing OWL"
$builddir/$uut $uut_params

#strip timestamp from dci
#sed 's/\([^\t]*\)\t\(.*\)/\2/' $workdir/test-falcon-dci.csv > $workdir/stripped-falcon-dci.csv
#sed 's/\([^\t]*\)\t\(.*\)/\2/' $templatedir/template-falcon-dci.csv > $workdir/template-falcon-dci.csv

cmp $workdir/test-owl-dci.csv $templatedir/template-owl-dci.csv
cmp $workdir/test-owl-stats.csv $templatedir/template-owl-stats.csv

echo "Done"
