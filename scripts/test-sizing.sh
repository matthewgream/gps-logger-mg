
test_post_sizing()
{
	test_assert_core_check
	test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"
	test_assert_file_string "$fil_tmp" "assert" 0
	mv -f "$dir_dst_staging/GPS00000.TXT" $dir_dst_outputs/$1 1>&2
}


# TEST: sizing
test_init "test-sizing"

# create a nmea file with all sentences!
f=$dir_dst_staging/all.txt
(
	test_loop "init!"
	for i in $dir_dst_samples/*.txt; do cat $i >> $f; done
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=nmea --cfg_format_nmea_sentences=rmc,gga,gll,vtg,gsv,gsa,zda --cfg_compress=false 2>&1 | tee $fil_tmp
	test_assert_core_check
	test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"
	test_assert_file_string "$fil_tmp" "assert" 0
	mv -f "$dir_dst_staging/GPS00000.TXT" "$f"
) >> $dir_dst_results/$test_name.txt

# create each of the output types
(
	# raw
	test_loop "all-c0-raw"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=raw --cfg_compress=false 2>&1 | tee $fil_tmp
	test_post_sizing "all-c0-raw"
	test_loop "all-c1-raw"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=raw --cfg_compress=true 2>&1 | tee $fil_tmp
	test_post_sizing "all-c1-raw"

	# nmea, gga
	test_loop "all-c0-nmea"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=nmea --cfg_format_nmea_sentences=gga --cfg_compress=false 2>&1 | tee $fil_tmp
	test_post_sizing "all-c0-nmea"
	test_loop "all-c1-nmea"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=nmea --cfg_format_nmea_sentences=gga --cfg_compress=true 2>&1 | tee $fil_tmp
	test_post_sizing "all-c1-nmea"

	# kml
	test_loop "all-c0-kml"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=kml --cfg_compress=false 2>&1 | tee $fil_tmp
	test_post_sizing "all-c0-kml"
	test_loop "all-c1-kml"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=kml --cfg_compress=true 2>&1 | tee $fil_tmp
	test_post_sizing "all-c1-kml"

	# csv-txt
	test_loop "all-c0-csvt"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=csv --cfg_format_csv_content=lon,lat,alt --cfg_format_csv_encoding=text --cfg_compress=false 2>&1 | tee $fil_tmp
	test_post_sizing "all-c0-csvt"
	test_loop "all-c1-csvt"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=csv --cfg_format_csv_content=lon,lat,alt --cfg_format_csv_encoding=text --cfg_compress=true 2>&1 | tee $fil_tmp
	test_post_sizing "all-c1-csvt"

	# csv-bin
	test_loop "all-c0-csvb"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=csv --cfg_format_csv_content=lon,lat,alt --cfg_format_csv_encoding=binary --cfg_compress=false 2>&1 | tee $fil_tmp
	test_post_sizing "all-c0-csvb"
	test_loop "all-c1-csvb"
	$exe_gps -d $dir_dst_staging -f $f -- --cfg_mode=file --cfg_file_buffer_size=0 \
		--cfg_format=csv --cfg_format_csv_content=lon,lat,alt --cfg_format_csv_encoding=binary --cfg_compress=true 2>&1 | tee $fil_tmp
	test_post_sizing "all-c1-csvb"
) >> $dir_dst_results/$test_name.txt

