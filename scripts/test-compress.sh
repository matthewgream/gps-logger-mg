
# TEST: compress
test_init "test-compress"
for i in $dir_dst_samples/*.txt; do
	(
		test_loop $i; \
		\
		( $exe_enc < "$i" | $exe_dec > "$dir_dst_staging/compress-tmp" ) 2> $fil_tmp; \
		cat $fil_tmp; \
		test_assert_file_exists "$dir_dst_staging/compress-tmp"; \
		test_assert_file_string "$fil_tmp" "assert" 0; \
		test_assert_file_equals "$i" "$fil_tmp"; \
		rm -f "$dir_dst_staging/compress-tmp"; \
	) >> $dir_dst_results/$test_name.txt
done

