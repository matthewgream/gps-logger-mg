#!/bin/sh

##############################################################################################

dir_gps=/opt/source/gps_logger_mg

dir_exe=$dir_gps
dir_src_samples=$dir_gps/samples/ext
dir_dst=$dir_gps/samples/tst
dir_dst_samples=$dir_dst/src
dir_dst_outputs=$dir_dst/out
dir_dst_results=$dir_dst/dat
dir_dst_staging=$dir_dst/tmp

fil_tmp=$dir_dst_staging/temp.txt

exe_gps=$dir_gps/gps_logger
exe_dec=$dir_gps/gps_lzdecode
exe_enc=$dir_gps/gps_lzencode

##############################################################################################

test_setup()
{
	# make sources
	rm -rf $dir_dst_samples
	mkdir -p $dir_dst_samples
	count=1000
	for i in $dir_src_samples/*/*; do
		ln $i $dir_dst_samples/$count.txt
		count=$(( $count + 1 ))
	done
	echo built $(( $count - 1000 )) files

	# make outputs
	rm -rf $dir_dst_results
	mkdir -p $dir_dst_results
	rm -rf $dir_dst_outputs
	mkdir -p $dir_dst_outputs
	rm -rf $dir_dst_staging
	mkdir -p $dir_dst_staging

	# tidy
	rm -f $exe_gps.core
}

test_init()
{
	test_name=$*
	rm -rf $dir_dst_staging/*
	echo "**** test-init: $test_name ****" 1>&2 | tee $dir_dst_results/$test_name.txt
}

test_loop()
{
	echo "**** test-loop: $test_name - $1 ****" 1>&2
}

test_archive()
{
	n=`echo $2 | sed -Ee 's/^[^0-9]*//g'`
	mv -f $3 $dir_dst_outputs/$1-$n 1>&2
}

##############################################################################################

test_assert_core_check()
{
	if [ -f $exe_gps.core -o -f `basename $exe_gps.core` ]; then
		echo "assert: test_assert_core_check: core detected" 1>&2
		ls -l $exe_gps.core
		rm -f $exe_gps.core
	fi
}

test_assert_file_exists()
{
	if [ ! -f $1 ]; then
		echo "assert: test_assert_file_exists: $1 does not exist!" 1>&2
	fi
}

test_assert_file_equals()
{
	s=$dir_dst_staging/src-tmp
	d=$dir_dst_staging/dst-tmp
	tr -d '\015' < $1 | sed '/^$/d' > $s
	tr -d '\015' < $2 | tr -d '\015' | sed '/^$/d' > $d
	if [ `cmp -s -z $s $d` ]; then
		echo "assert: test_assert_file_equals: '$1' and '$2' differ" 1>&2
		ls -l $1 $2
		ls -l $s $d
	fi
	rm -f $s $d
}

test_assert_file_string()
{
	c=`grep -c $2 $1`
	if [ $c -ne $3 ]; then
		echo "assert: test_assert_file_string: '$1' match '$2' count $c (not '$3')" 1>&2
	fi
}

##############################################################################################

test_assert_file_compress_check()
{
	t=$dir_dst_staging/dec-tmp
	$exe_dec < $1 > $fil_tmp 2> $t
	cat $t; rm -f $t
	test_assert_file_exists "$fil_tmp"
	test_assert_file_equals "$fil_tmp" "$1"
}

##############################################################################################

test_assert_file_format_raw()
{
		test_assert_file_equals "$1" "$2"
}

test_assert_file_format_nmea()
{
		# regexp on nmea format
}

test_assert_file_format_kml()
{
		# rexgexp on kml format
		# compare w/ gpsconv
}

test_assert_file_format_csv_txt()
{
		# regexp on csv format
		# compare w/ gpsconv
}

test_assert_file_format_csv_bin()
{
		# unpack & compare to txt
}

##############################################################################################

test_setup
. ./test-format.sh
. ./test-mode-file.sh
. ./test-sizing.sh
. ./test-compress.sh

