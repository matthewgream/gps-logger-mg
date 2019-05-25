#!/bin/sh

EXE=../gps_logger
SRC=raw/NMEA-RAW.TXT
IMG=fat.img

show_files()
{
	mdconfig -a -t vnode -f $* -u 1
	mount -t msdosfs /dev/md1s1 /mnt/fat
	ls -l /mnt/fat
	umount /mnt/fat
	mdconfig -d -u 1
}

bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f test -- --cfg_format=nmea --cfg_format_nmea_sentences=gga --cfg_compress=false --cfg_file_buffer_size=24576 > all-test.log
show_files $IMG >> all-test.log

bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=nmea --cfg_format_nmea_sentences=gga --cfg_compress=false --cfg_file_buffer_size=24576 > all-nmea-c0.log
show_files $IMG >> all-nmea-c0.log
bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=nmea --cfg_format_nmea_sentences=gga --cfg_compress=true --cfg_file_buffer_size=16384 > all-nmea-c1.log
show_files $IMG >> all-nmea-c1.log

bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=kml --cfg_compress=false --cfg_file_buffer_size=24576 > all-kml-c0.log
show_files $IMG >> all-kml-c0.log
bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=kml --cfg_compress=true --cfg_file_buffer_size=16384 > all-kml-c1.log
show_files $IMG >> all-kml-c1.log

bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=csv --cfg_format_csv_encoding=text --cfg_format_csv_content=lon,lat,alt --cfg_compress=false --cfg_file_buffer_size=24576 > all-csvt-c0.log
show_files $IMG >> all-csvt-c0.log
bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=csv --cfg_format_csv_encoding=text --cfg_format_csv_content=lon,lat,alt --cfg_compress=true --cfg_file_buffer_size=16384 > all-csvt-c1.log
show_files $IMG >> all-csvt-c1.log

bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=csv --cfg_format_csv_encoding=binary --cfg_format_csv_content=lon,lat,alt --cfg_compress=false --cfg_file_buffer_size=24576 > all-csvb-c0.log
show_files $IMG >> all-csvb-c0.log
bzcat fat_256m.img.bz2 > $IMG
$EXE -i $IMG -f $SRC -- --cfg_format=csv --cfg_format_csv_encoding=binary --cfg_format_csv_content=lon,lat,alt --cfg_compress=true --cfg_file_buffer_size=16384 > all-csvb-c1.log
show_files $IMG >> all-csvb-c1.log

tail all-*.log > all.log

