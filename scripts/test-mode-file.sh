
# TEST: mode: file-buffer
test_init "test-mode-file-buffer"
for i in `ls $dir_dst_samples/*.txt | head -1`; do
	(
		test_loop $i; \
		\
		$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=0 \
			--cfg_format=kml --cfg_compress=false 2>&1 | tee $fil_tmp; \
		test_assert_core_check; \
		test_assert_file_exists "$dir_dst_staging/GPS00000.TXT"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		\
		n=0; \
		while [ $n -le 24768 ]; do \
			echo "*** using buffer size = $n"; \
			$exe_gps -d $dir_dst_staging -f $i -- --cfg_mode=file --cfg_file_buffer_size=$n \
				--cfg_format=kml --cfg_compress=false 2>&1 | tee $fil_tmp; \
			test_assert_core_check; \
			test_assert_file_exists "$dir_dst_staging/GPS00001.TXT"; \
			test_assert_file_string "$fil_tmp" "assert" 0; \
			test_assert_file_equals "$dir_dst_staging/GPS00000.TXT" "$dir_dst_staging/GPS00001.TXT"; \
			rm -f "$dir_dst_staging/GPS00001.TXT"; \
			p=`od -t u1 -N 4 /dev/random | sed 's/^[0-9]*[ \t]*//' | head -1 | sed -E -e 's/[ ]*$//' -e 's/[ ]+/+/g'`; \
			n=$(($n + $p)); \
		done; \
	) >> $dir_dst_results/$test_name.txt
done

