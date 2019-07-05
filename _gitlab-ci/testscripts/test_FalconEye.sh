#!/bin/bash 

templatedir="../testdata"
builddir=$1
workdir=$2

uut="src/FalconEye"
uut_params="-i $templatedir/template-iq.bin -p 50 -P 2 -c 392 -n 1000 -D $workdir/test-falcon-dci.csv -E $workdir/test-falcon-stats.csv"

#exit immediately if any command fails
set -e

echo "Testing FALCON Eye"
$builddir/$uut $uut_params

#strip timestamp from dci
#sed 's/\([^\t]*\)\t\(.*\)/\2/' $workdir/test-falcon-dci.csv > $workdir/stripped-falcon-dci.csv
#sed 's/\([^\t]*\)\t\(.*\)/\2/' $templatedir/template-falcon-dci.csv > $workdir/template-falcon-dci.csv

#strip timestamp (first column) and histogram counter (last column) from dci
cat $workdir/test-falcon-dci.csv | cut -f2- | rev | cut -f2- | rev > $workdir/stripped-test-falcon-dci.csv
cat $templatedir/template-falcon-dci.csv | cut -f2- | rev | cut -f2- | rev > $workdir/stripped-template-falcon-dci.csv

#copy dci to tmp (e.g. for updating tests)
cp $workdir/test-falcon-dci.csv /tmp/tmp-host
cp $workdir/stripped-test-falcon-dci.csv /tmp/tmp-host
cp $templatedir/template-falcon-dci.csv /tmp/tmp-host
cp $workdir/stripped-template-falcon-dci.csv /tmp/tmp-host

#copy stats to tmp
cp $workdir/test-falcon-stats.csv /tmp/tmp-host
cp $templatedir/template-falcon-stats.csv /tmp/tmp-host

echo "Comparing DCI"
#diff $workdir/stripped-falcon-dci.csv $workdir/template-falcon-dci.csv
cmp $workdir/stripped-test-falcon-dci.csv $workdir/stripped-template-falcon-dci.csv
echo "Comparing stats"
cmp $workdir/test-falcon-stats.csv $templatedir/template-falcon-stats.csv

echo "Done"
