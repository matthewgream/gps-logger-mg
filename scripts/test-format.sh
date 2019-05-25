
# TEST: format: raw
test_init "test-format-raw"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=raw --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_format_raw "$i" "$dir_dst_staging/GPS00000.TXT"; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=raw --cfg_compress=true 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_compress_check "$dir_dst_staging/GPS00001.TXT" "$dir_dst_staging/GPS00000.TXT"; \
		\
		test_archive "raw-c0" $i "$dir_dst_staging/GPS00000.TXT"; \
		test_archive "raw-c1" $i "$dir_dst_staging/GPS00001.TXT"; \
	) >> $dir_dst_results/$test_name.txt
done

# TEST: format: nmea
test_init "test-format-nmea"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=nmea --cfg_format_nmea_sentences=rmc,gga,zda --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_format_nmea "$i" "$dir_dst_staging/GPS00000.TXT"; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=nmea --cfg_format_nmea_sentences=rmc,gga,zda --cfg_compress=true 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_compress_check "$dir_dst_staging/GPS00001.TXT" "$dir_dst_staging/GPS00000.TXT"; \
		\
		test_archive "nmea-c0" $i "$dir_dst_staging/GPS00000.TXT"; \
		test_archive "nmea-c1" $i "$dir_dst_staging/GPS00001.TXT"; \
	) >> $dir_dst_results/$test_name.txt
done

# TEST: format: kml
test_init "test-format-kml"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=kml --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_format_kml "$i" "$dir_dst_staging/GPS00000.TXT"; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=kml --cfg_compress=true 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_compress_check "$dir_dst_staging/GPS00001.TXT" "$dir_dst_staging/GPS00000.TXT"; \
		\
		test_archive "kml-c0" $i "$dir_dst_staging/GPS00000.TXT"; \
		test_archive "kml-c1" $i "$dir_dst_staging/GPS00001.TXT"; \
	) >> $dir_dst_results/$test_name.txt
done

# TEST: format: csv-txt
test_init "test-format-csv-txt"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=csv --cfg_format_csv_content=lon,lat,alt,tim --cfg_format_csv_encoding=text --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_format_csv_txt "$i" "$dir_dst_staging/GPS00000.TXT"; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=csv --cfg_format_csv_content=lon,lat,alt,tim --cfg_format_csv_encoding=text --cfg_compress=true 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_compress_check "$dir_dst_staging/GPS00001.TXT" "$dir_dst_staging/GPS00000.TXT"; \
		\
		test_archive "csv-t-c0" $i "$dir_dst_staging/GPS00000.TXT"; \
		test_archive "csv-t-c1" $i "$dir_dst_staging/GPS00001.TXT"; \
	) >> $dir_dst_results/$test_name.txt
done

# TEST: format: csv-bin
test_init "test-format-csv-bin"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=csv --cfg_format_csv_content=lon,lat,alt,tim --cfg_format_csv_encoding=binary --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_format_csv_bin "$i" "$dir_dst_staging/GPS00000.TXT"; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=csv --cfg_format_csv_content=lon,lat,alt,tim --cfg_format_csv_encoding=binary --cfg_compress=true 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_compress_check "$dir_dst_staging/GPS00001.TXT" "$dir_dst_staging/GPS00000.TXT"; \
		\
		test_archive "csv-b-c0" $i "$dir_dst_staging/GPS00000.TXT"; \
		test_archive "csv-b-c1" $i "$dir_dst_staging/GPS00001.TXT"; \
	) >> $dir_dst_results/$test_name.txt
done

# XXX graph ...

