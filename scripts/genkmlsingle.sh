#!/bin/sh

DATE=`date +"%Y%m%d"`

cat << EOFH
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://earth.google.com/kml/2.0">
    <Placemark>
      <name>gps - $DATE</name>
      <description>gps - $DATE</description>
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
cat $*
cat << EOFT
        </coordinates>
      </LineString>
    </Placemark>
</kml>
EOFT

exit 0
