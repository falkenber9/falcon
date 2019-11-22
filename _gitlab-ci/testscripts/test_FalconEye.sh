#!/bin/bash 

#exit immediately if any command fails
set -e

echo -e "\n********************* STARTING TEST *********************\n"

templatedir="../testdata"
builddir=$1
workdir=$2

uut="src/FalconEye"

load_subtest() {
	source "$templatedir/$subtest/args.sh"
}

load_variant() {
	input_dir=$templatedir/$subtest
	input_iq=$basename-$iq_suffix
	input_dci=$basename$variant-$dci_suffix
	input_stats=$basename$variant-$stats_suffix

	output_dir=$workdir
	output_dci=test$variant-$dci_suffix
	output_stats=test$variant-$stats_suffix
}

strip_outputs_falcon() {
	echo "Stripping outputs"
	#strip timestamp (first column) and histogram counter (last column) from dci
	cat $output_dir/$output_dci | cut -f2- | rev | cut -f4- | rev > $output_dir/stripped-$output_dci
	cat $input_dir/$input_dci | cut -f2- | rev | cut -f4- | rev > $output_dir/stripped-$input_dci
}

store_input_output_falcon() {
	echo "Storing inputs and outputs"
	#copy dci to tmp (e.g. for updating tests)
	mkdir -p /tmp/tmp-host/$subtest
	cp $output_dir/$output_dci /tmp/tmp-host/$subtest
	cp $output_dir/stripped-$output_dci /tmp/tmp-host/$subtest
	cp $input_dir/$input_dci /tmp/tmp-host/$subtest
	cp $output_dir/stripped-$input_dci /tmp/tmp-host/$subtest

	#copy stats to tmp
	cp $output_dir/$output_stats /tmp/tmp-host/$subtest
	cp $input_dir/$input_stats /tmp/tmp-host/$subtest
}

run_falcon() {
	echo "Running subtest '$subtest', variant '$variant': $uut $uut_params"
	$builddir/$uut $uut_params
}

postprocess_falcon() {
	echo "Postprocessing"
	strip_outputs_falcon
	store_input_output_falcon
}

validate_falcon() {
	echo "Comparing DCI"
	cmp $output_dir/stripped-$output_dci $output_dir/stripped-$input_dci

	echo "Comparing stats"
	cmp $output_dir/$output_stats $input_dir/$input_stats
}

######### TESTS ##########

subtest="normal-10-MHz"
load_subtest

variant="-falcon"
load_variant

uut_params="-i $input_dir/$input_iq -p $nof_prb -P $antenna_ports -c $cell_id -n $nof_subframes -D $output_dir/$output_dci -E $output_dir/$output_stats -r"
run_falcon

postprocess_falcon
validate_falcon

echo "Passed"

##########################

variant="-falcon-histogram"
load_variant

uut_params="-i $input_dir/$input_iq -p $nof_prb -P $antenna_ports -c $cell_id -n $nof_subframes -D $output_dir/$output_dci -E $output_dir/$output_stats -r -H"
run_falcon

postprocess_falcon
validate_falcon

echo "Passed"

##########################

subtest="busy-15-MHz"
load_subtest

variant=""
load_variant

uut_params="-i $input_dir/$input_iq -p $nof_prb -P $antenna_ports -c $cell_id -D $output_dir/$output_dci -E $output_dir/$output_stats -r"
run_falcon

postprocess_falcon
validate_falcon

echo "Passed"

##########################

variant="-histogram"
load_variant

uut_params="-i $input_dir/$input_iq -p $nof_prb -P $antenna_ports -c $cell_id -D $output_dir/$output_dci -E $output_dir/$output_stats -r -H"
run_falcon

postprocess_falcon
validate_falcon

echo "Passed"

##########################
##########################

echo "Success"
