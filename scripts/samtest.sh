#!/bin/sh

dir_gps=/opt/source/gps_logger_mg

dir_exe=$dir_gps
dir_src_samples=$dir_gps/samples/ext
dir_dst=$dir_gps/results/samtest
dir_dst_samples=$dir_dst/samples
dir_dst_outputs=$dir_dst/outputs
dir_dst_results=$dir_dst/results

# make sources
rm -rf $dir_dst_samples
mkdir -p $dir_dst_samples
count=1000
for i in $dir_src_samples/*/*; do
	ln $i $dir_dst_samples/$count.txt
	count=$(( $count + 1 ))
done
echo built $(( $count - 1000 )) files

# make results
rm -rf $dir_dst_results
mkdir -p $dir_dst_results
rm -rf $dir_dst_outputs
mkdir -p $dir_dst_outputs
for i in $dir_dst_samples/*; do

	echo "****" $i "****"
	$dir_exe/gps_logger -d $dir_dst_outputs -f $i -- \
		--cfg_mode=file --cfg_format=nmea --cfg_file_buffer_size=0 --cfg_format_nmea_sentences=gga \
		>> $dir_dst_results/output.txt

done


