#!/bin/sh

DEC_C=../../gps_lzdecode
DEC_P=../../scripts/unpack.pl
EXE=../../gps_logger
SRC=$1
TMP=/tmp/gpsdec.tmp
LOG=/tmp/gpsdec.log
PRE=$2

check()
{
	cmp $1 $2
	if [ $? -ne 0 ]; then
		echo \!\!\! compare failed: \'$1\' to \'$2\' 1>&2
	fi
}

echo \*\*\* build formats and options ... using \'$EXE\' and input \'$SRC\' for \'$PRE\'
echo \*\*\* check ... using \'$DEC_C\' and \'$DEC_P\'
( 
	echo build raw ...
    $EXE -d . -f $SRC -- --cfg_format=raw --cfg_compress=false && \
	    mv -f GPS00000.TXT $PRE-raw.txt
    $EXE -d . -f $SRC -- --cfg_format=raw --cfg_compress=true && \
	    mv -f GPS00000.TXT $PRE-raw-compress.txt

	echo build nmea ...
    $EXE -d . -f $SRC -- --cfg_format=nmea --cfg_compress=false && \
	    mv -f GPS00000.TXT $PRE-nmea.txt
    $EXE -d . -f $SRC -- --cfg_format=nmea --cfg_compress=true && \
	    mv -f GPS00000.TXT $PRE-nmea-compress.txt

	echo build kml ...
    $EXE -d . -f $SRC -- --cfg_format=kml --cfg_compress=false && \
	    mv -f GPS00000.TXT $PRE-kml.txt
    $EXE -d . -f $SRC -- --cfg_format=kml --cfg_compress=true && \
	    mv -f GPS00000.TXT $PRE-kml-compress.txt

	echo build csv-text ...
    $EXE -d . -f $SRC -- --cfg_format=csv --cfg_compress=false --cfg_format_csv_encoding=text --cfg_format_csv_content=lon,lat,alt,tim && \
	    mv -f GPS00000.TXT $PRE-csv-text.txt
    $EXE -d . -f $SRC -- --cfg_format=csv --cfg_compress=true --cfg_format_csv_encoding=text --cfg_format_csv_content=lon,lat,alt,tim && \
	    mv -f GPS00000.TXT $PRE-csv-text-compress.txt

	echo build csv-binary ...
    $EXE -d . -f $SRC -- --cfg_format=csv --cfg_compress=false --cfg_format_csv_encoding=binary --cfg_format_csv_content=lon,lat,alt,tim && \
	    mv -f GPS00000.TXT $PRE-csv-binary.txt
    $EXE -d . -f $SRC -- --cfg_format=csv --cfg_compress=true --cfg_format_csv_encoding=binary --cfg_format_csv_content=lon,lat,alt,tim && \
	    mv -f GPS00000.TXT $PRE-csv-binary-compress.txt

    mv -f GPSCONFG.TXT gpsconfg.txt
	rm -f $TMP
) 2>&1 > $LOG | grep -v -i -e DOWN -e gps_logger -e ''

(
	echo check raw ...
    $DEC_C < $PRE-raw-compress.txt > $TMP && check $PRE-raw.txt $TMP
	echo check nmea ...
    $DEC_C < $PRE-nmea-compress.txt > $TMP && check $PRE-nmea.txt $TMP
	echo check kml ...
    $DEC_C < $PRE-kml-compress.txt > $TMP && check $PRE-kml.txt $TMP
	echo check csv-text ...
    $DEC_C < $PRE-csv-text-compress.txt > $TMP && check $PRE-csv-text.txt $TMP
	echo check csv-binary ...
    $DEC_C < $PRE-csv-binary-compress.txt > $TMP && check $PRE-csv-binary.txt $TMP
	rm -f $TMP
) 2>&1 >> $LOG | grep -v -i -e copyright -e gps_logger -e read

(
	echo check csv-binary ...
	$DEC_P -n -e lon,lat,alt,tim < $PRE-csv-binary.txt > $TMP && check $PRE-csv-text.txt $TMP
	rm -f $TMP
) 2>&1 >> $LOG | grep -v -i -e copyright -e gps_logger

(
cat << EOFH
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://earth.google.com/kml/2.0">
    <Placemark>
      <name>GPS Logger MG - Example KML ($PRE) - 20070327</name>
      <description>GPS Logger MG - Example KML ($PRE) - 20070327</description>
      <Style>
        <LineStyle>
          <width>4</width>
        </LineStyle>
      </Style>
      <LineString>
        <tessellate>1</tessellate>
        <altitudeMode>clampToGround</altitudeMode>
        <coordinates>
EOFH
cat $PRE-kml.txt
cat << EOFT
	    </coordinates>
      </LineString>
    </Placemark>
</kml>
EOFT
) > $PRE.kml

exit 0
