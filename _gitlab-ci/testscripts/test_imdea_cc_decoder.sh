#!/bin/bash 

echo -e "\n********************* STARTING TEST *********************\n"

templatedir="../testdata"
builddir=$1
workdir=$2

uut="benchmark/imdeaowl/imdea_cc_decoder"

subtest="normal-10-MHz"
source "$templatedir/$subtest/args.sh"
variant="-owl"

input_dir=$templatedir/$subtest
input_iq=$basename-$iq_suffix
input_dci=$basename$variant-$dci_suffix
input_stats=$basename$variant-$stats_suffix

output_dir=$workdir
output_dci=test$variant-$dci_suffix
output_stats=test$variant-$stats_suffix

uut_params="-i $input_dir/$input_iq -p $nof_prb -P $antenna_ports -c $cell_id -n $nof_subframes -D $output_dir/$output_dci -E $output_dir/$output_stats -d"

#exit immediately if any command fails
set -e

echo "Running subtest '$subtest', variant '$variant': $uut $uut_params"
$builddir/$uut $uut_params

#copy dci to tmp (e.g. for updating tests)
mkdir -p /tmp/tmp-host/$subtest
cp $output_dir/$output_dci /tmp/tmp-host/$subtest
cp $input_dir/$input_dci /tmp/tmp-host/$subtest

#copy stats to tmp
cp $output_dir/$output_stats /tmp/tmp-host/$subtest
cp $input_dir/$input_stats /tmp/tmp-host/$subtest

echo "Comparing DCI"
cmp $output_dir/$output_dci $input_dir/$input_dci

echo "Comparing stats"
cmp $output_dir/$output_stats $input_dir/$input_stats

echo "Success"
