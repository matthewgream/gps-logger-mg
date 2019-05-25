#!/usr/bin/perl
use strict;
use Getopt::Std;
use Time::Local;
use Time::localtime;
use File::Basename;
#use File::DosGlob 'glob';
#use Time::gmtime;
use Math::Trig;
use POSIX qw(strftime);
#use Date::Calc qw(Day_of_Week Day_of_Week_to_Text);

my($version) = "0.9.0";

my($options) = "gpsconv [options] <file>
   Author: Raymond Choc, gpsconv(at)melibokus-biker.de
   Translater for track- and waypoint-formats like GPX, KML, TOP50-overlay, PCX5, G7T, Magellan-SD-Card-Format, ...
   Options:
     -v version info
     -q be quiet
     -h print this help
     -H format, detail information for a format, e.g. -H nmea 
        (choose one of the formats, listed with -f)
     -f format list of supported formats 
     -i format of the input file: choose one of the formats, 
        listed with -f and marked with (i)
     -o format of the output file, choose one of the formats, 
        listed with -f and marked with (o)
     -a analsye the readed track 
        (distance and distance over ground, altitude up and down in meters).
     -d destination file name (or path), usefull if you read more than one file
        and write them to a multi-track-format or a different directory 
     -r reverse track 
     -n write tracks to one file or each track to separate files 
        (only supported by multi-track-formats): 
        '-n 1' for separate file for each track 
        (default for the formats: pcxg,gcxm,g7t,ovl),
        '-n n' if destination format support it write all tracks to one file 
        (default for the formats: kml,kmlr,csv,gpx,garm)
     -j for joining 'ACTIVE LOG'-tracksegments (default is '-j d'):
        '-j n' no joinig tracksegments,
        '-j a' join all tracksegments,
        '-j d' join all tracksegments with the same date,
        '-j <time in minutes>' join segments if the time difference between the
        segments is smaller than the value of '-j' in minutes
     -s for split one track to tracks with a maximum number of trackpoints:
        '-s <max. no. of points[,no. of overlaped points]>' 
        split a track to tracks with a limited number of trackpoints,
        specify optional the no. of overlaping points\n";

my($bequiet) = 0;
my($ifname,$ofname,$destFilename);
my($srcFormat,$destFormat);
my(@tracks, @wpts);
my($joinTime)='d';
my($FILEMODE_SINGLETRACK)=1;
my($FILEMODE_MULTITRACK)=0;
my($singleFileMode)=$FILEMODE_SINGLETRACK;
my($reverse)=0;
my($splitMaxNoOfTrackPts)=0;

die("Option error!\n$options")
    unless getopts('hH:vqi:o:fe:ad:n:j:rs:');

if (defined($Getopt::Std::opt_e)) {
	my($logFile) = $Getopt::Std::opt_e;
	if($logFile ne ""){
	  open(STDERR,">>$logFile");
	}
}if (defined($Getopt::Std::opt_v)) {
    print STDERR "gpsconv Version $version\n";
    exit;
}if (defined($Getopt::Std::opt_h)) {
    print STDERR ($options);
    exit 0;
}if (defined($Getopt::Std::opt_H)) {
	my($helpFormat) = $Getopt::Std::opt_H;
	if($helpFormat =~ /^(exp|mag)/){
  	&help_mag;
	} elsif($helpFormat =~ /^(tk|mm)/){
  	&help_tk_mm;
	} elsif($helpFormat =~ /^(pcx|pcxm|pcxg)/){
  	&help_pcx;
  }elsif($helpFormat eq "fug"){
  	&help_fug;
  }elsif($helpFormat eq "ovl"){
  	&help_ovl;
  }elsif($helpFormat eq "garm"){
  	&help_garm;
  }elsif($helpFormat eq "g7t"){
  	&help_garm;
  }elsif($helpFormat eq "nmea"){
  	&help_nmea;
  }elsif($helpFormat eq "kml"){
  	&help_kml;
  }elsif($helpFormat eq "kmlr"){
  	&help_kmlr;
  }elsif($helpFormat eq "csv"){
  	&help_csv;
  }elsif($helpFormat eq "gpx"){
  	&help_gpx;
	}else{
  	print STDERR "Unkown format '$helpFormat'";
  }
	exit 0;
}
if (defined($Getopt::Std::opt_f)) {
	print STDERR "exp  T,W(i,o) Magellan-NMEA-Format, used by Magellan on the SD-Card\n";
	print STDERR "              of the eXplorist\n";
	print STDERR "mag  T,W(i,o) Magellan-NMEA-Format, used by Magellan on the SD-Card\n";
	print STDERR "              of the Meridian and by the following Mapsend Products:\n";
	print STDERR "              DR [EU|NA], WorldWideBaseMap and Topo 3D\n";
	print STDERR "nmea T(i,o)   NMEA 1803-Format, used by same PocketPC's\n";
	print STDERR "ovl  T(i,o)   TOP50-ASCII-Overlay, used also by MagicMaps 3D\n";
	print STDERR "tk   T(i,o)   KOMPASS and AlpenVerein-Format, used also by Fugawi\n";
	print STDERR "mm   T(i,o)   identical to the KOMPASS-Format, used as\n";
	print STDERR "              ASCII-Text-Format from MagicMaps 3D\n";
	print STDERR "fug  T,W(i,o) Fugawi-Text-Format\n";
	print STDERR "pcx  T,W(i)   PCX5-Format, can read all versions and variants of PCX5\n";
	print STDERR "pcxg T,W(o)   PCX5 2.08-Format(use [+|-]grad.fraction), used also by\n";
	print STDERR "              TTQV, Fugawi(read only), GPSTrackmaker, G7ToWin\n";
	print STDERR "pcxm T,W(o)   PCX5 2.09-Format (use [N|S|W|E]grad min.fraction), used\n";
	print STDERR "              also by TTQV, G7ToWin, Garmin Mapsource\n";
	print STDERR "g7t  T(i,o)   g7t-Format, used by G7ToWin and e.g. as upload-format\n";
	print STDERR "              for www.gps-info.de, www.mtb.tourfinder.net\n";
	print STDERR "garm T(i,o)   Garmin-Mapsource-Text-Export-Format, just (all)\n";
	print STDERR "              Active Log will be read, can be read by GPS-Tack-Analyse\n";
	print STDERR "kml  T,W(i,o) Google Earth-Text-Format\n";
	print STDERR "kmlr T,W(o)   Google Earth-Text-Format (raw, no header or footer)\n";
	print STDERR "csv  T,W(o)   CSV fields of lon,lat,alt,tim\n";
	print STDERR "gpx  T,W(i,o) GPX (GPS Exchange Format)\n";
  exit 0;
}
if (defined($Getopt::Std::opt_i)) {
  $srcFormat = $Getopt::Std::opt_i;
	if ($srcFormat !~ /^(nmea|exp|mag|ovl|tk|mm|fug|pcx|garm|g7t|gpx|kml)/) {
	    print STDERR "unkown input Format $srcFormat\n";
	    exit;
	}
}
if (defined($Getopt::Std::opt_o)) {
	$destFormat = $Getopt::Std::opt_o;
	if ($destFormat !~ /^(nmea|exp|mag|ovl|tk|mm|fug|pcxg|pcxm|garm|g7t|kml|kmlr|csv|gpx)/) {
		print STDERR "unkown output Format $destFormat\n";
		exit;
	}
	if($destFormat =~ /^(garm|kml|kmlr|csv|gpx)/) {
		$singleFileMode = $FILEMODE_MULTITRACK;
	}
}
if (defined($Getopt::Std::opt_d)) {
	$destFilename = $Getopt::Std::opt_d;
}

if (defined($Getopt::Std::opt_q)) {
	$bequiet = 1;
}
if (defined($Getopt::Std::opt_n)) {
	if($Getopt::Std::opt_n eq 1){
		$singleFileMode = $FILEMODE_SINGLETRACK;
	} elsif($Getopt::Std::opt_n eq 'n') {
		$singleFileMode = $FILEMODE_MULTITRACK;
	} else{
		print STDERR "unknown value '$Getopt::Std::opt_n' for option '-n', only '-n 1' or '-n n' are allowed\n";
		exit;
	}
}
if (defined($Getopt::Std::opt_j)) {
  $joinTime = $Getopt::Std::opt_j;
  if($joinTime !~ /^(a|n|d|[0-9]*)$/){
		print STDERR "unknown value '$Getopt::Std::opt_j' for option '-j', only 'n','a','d' or a number are allowed\n";
		exit;
	}  
}

if (defined($Getopt::Std::opt_s)) {
  $splitMaxNoOfTrackPts = $Getopt::Std::opt_s;
  if($splitMaxNoOfTrackPts !~ /^([0-9]*),?([0-9]*)?$/){
		print STDERR "unknown value '$Getopt::Std::opt_s' for option '-s', only numbers are allowed\n";
		exit;
	}  
}

if (!defined($ARGV[0])) {
	die("No file to convert!\n$options");
}

my(@fileList);
foreach (@ARGV){
	my($filename,$path) = fileparse($_);
	chdir($path);
#	print STDERR "file: '$filename'\n";
	$filename =~ s/ /?/g;
#	print STDERR "file: '$filename'\n";
	push @fileList, map {$path.$_} glob $filename;
}
foreach $ifname (@fileList){
#	$ifname = ~ s/_/ /g;	
	unless (open(INPUT,"<$ifname")) {
	    die "Cannot open '$ifname' for reading";
	}
	
	if (!$bequiet) {
	    print STDERR "Reading track point data or waypoints from the $srcFormat-File '$ifname' ...\n";
	}
	#### read source track ##########################
	if($srcFormat =~ /^(pcx)/){
		push @tracks, readPCX5($ifname,$joinTime,\@wpts);
	} elsif($srcFormat =~ /^(g7t)/){
		push @tracks, readG7T($ifname,$joinTime,\@wpts);
	}elsif($srcFormat =~ /^(garm)/){
		push @tracks, readGarmin($ifname,$joinTime,\@wpts);
	}elsif($srcFormat =~ /^(gpx)/){
		push @tracks, readGpx($ifname,$joinTime,\@wpts);
	}elsif($srcFormat =~ /^(ovl)/){
		push @tracks, readOverlay($ifname);
	}elsif($srcFormat =~ /^(kml)/){
		push @tracks, readKml($ifname,\@wpts);
	} else{
		my($track) = readSingleTrack($srcFormat,$ifname,\@wpts);
		if($track){
			push @tracks, $track;
		}
	}
	close INPUT;
}
if(@tracks == 0 && @wpts == 0){
	print STDERR "No tracks or waypoints as '$srcFormat' were readed";
	exit 0;
}

if (!$bequiet) {
    print STDERR "Reading all ".($#tracks+1)." track/s and ".($#wpts+1)
    						." waypoints from ".($#fileList+1)." file/s\n";
		print STDERR "Writing now new track/waypoints-file/s with the format '$destFormat' ...\n";
}

#### reverse tracks ##########################
if(defined($Getopt::Std::opt_r)){
	@tracks = reverseTracks(@tracks);	
}

#### split tracks ##########################
if(defined($Getopt::Std::opt_s)){
	@tracks = splitTracks(\@tracks, $splitMaxNoOfTrackPts);	
}

#### analyse the tracks ##########################
if(defined($Getopt::Std::opt_a) || $destFormat =~ /^(garm|gpx)/){
	@tracks = analyseTrack(@tracks);	
}

#### write track to destination format ##########
if($destFormat =~ /^(kml|kmlr|csv|pcx|g7t|garm|gpx|ovl)/){
	if($singleFileMode && @tracks >1){
		my($track);
		foreach $track (@tracks){
			my(@tracks) = ($track);
			writeMultiTracks($destFormat,$singleFileMode, \@tracks, \@wpts);
		}
	} else{
		writeMultiTracks($destFormat,$singleFileMode, \@tracks, \@wpts);
	}
	if($destFormat =~ /^(pcx)/){
		writePCX5Wpts($destFormat,\@wpts);
	}
} else{
	writeSingleTracks($destFormat,\@tracks,\@wpts);
}
if (!$bequiet) {
  print STDERR "... done\n";
}
# END MAIN #

######### Subroutines ##########################

sub readSingleTrack{
	my($srcFormat,$ifname,$wptsRef) =@_; 
#	my ($wptsRef) = pop @_;
	my($track,@data,$dataRef);
	my($trkname,$path,$suffix) = fileparse($ifname,qw{\.\w*$});

#	print "readSingleTracks: ".@{$wptsRef}.", ".$wptsRef." \n";
	
	if($srcFormat =~ /^(exp|mag)/){
		@data = &readMagellan($wptsRef);
#		@data = @$dataRef;
	}elsif($srcFormat eq "nmea"){
		@data = &readNMEA;
	}elsif($srcFormat eq "fug"){
		@data = &readFugawi($wptsRef);
	}elsif($srcFormat =~ /^(tk|mm)/){
		@data = &readKompass;
	}
	if(@data != 0){
		$track={
			filename => $trkname,
			path => $path,
			data => \@data,
		};
	}
#	my(@datas) = @{$track->{data}};
#	print "R $track->{name}, Anzahl: $#datas, $#data\n";
#	print "readSingleTracks: ".@{$wptsRef}.", ".$wptsRef." \n";
	if(@data == 0 && @{$wptsRef} == 0){
		print STDERR "Can't proberly read this file as '$srcFormat'";
		exit 0;
	}
	$track;
}

sub writeSingleTracks{
	my($destFormat,$tracksRef,$wptsRef) =@_;
	my($track,@data);
	foreach $track (@{$tracksRef}){
		my($filename) = destFilename($track,$destFormat,$FILEMODE_SINGLETRACK,$destFilename);
		unless (open(OUT,">$filename")) {
	 	   die "Cannot open '$filename' for writing";
		}
		binmode(OUT);
		@data=@{$track->{data}};
	#	print "W $tracks[0]->{name}, Anzahl: $#data, $destFormat\n";
		if($destFormat =~ /^(tk|mm)/){
			writeKompass(@data);
		}elsif($destFormat =~ /^(exp|mag)/){
			writeMagellanTrack(@data);
		}elsif($destFormat eq "nmea"){
			writeNMEA(@data);
		}elsif($destFormat eq "fug"){
			writeFugawiTrack(@data);
		}		
		close OUT;
	}
	# write Waypoints
	if(@{$wptsRef}>0){
		my($filename) = destWptsFilename($destFormat,$destFilename);
		unless (open(OUT,">$filename")) {
			die "Cannot open '$filename' for writing waypoints";
	 	}
	 	binmode(OUT);
	# 	print "writeSingleTracks: ".@{$wptsRef}.", ".$wptsRef."\n";
		if($destFormat =~ /^(exp|mag)/){
			writeMagellanWpts($wptsRef);
		}elsif($destFormat eq "fug"){
			writeFugawiWpts($wptsRef);
		}else{
			unlink "$filename";
		}
	}
}

sub writeMultiTracks{
	my($destFormat,$singleFileMode,$tracksRef,$wptsRef) =@_;
	my (@tracks) = @{$tracksRef};
	if($destFormat eq "kmlr"){
		writeKmlr($singleFileMode,$tracksRef,$wptsRef);
	}elsif($destFormat eq "csv"){
		writeCSV($singleFileMode,$tracksRef,$wptsRef);
	}elsif($destFormat eq "kml"){
		writeKml($singleFileMode,$tracksRef,$wptsRef);
	}elsif($destFormat =~ /^(pcx)/){
		writePCX5Tracks($destFormat,$singleFileMode,@tracks);
	}elsif($destFormat eq "g7t"){
		writeG7T($singleFileMode,@tracks);
	}elsif($destFormat eq "garm"){
		writeGarmin($singleFileMode,@tracks);
	}elsif($destFormat eq "gpx"){
		writeGpx($singleFileMode,$tracksRef,$wptsRef);
	}elsif($destFormat eq "ovl"){
		writeOverlay($singleFileMode,@tracks);
	}
}

sub prepareNMEAChecksum {
	my($mess) = @_;
	my($csum) = 0;
	my($copy) = $mess;
	
	while ($copy ne "") {
		$csum = $csum ^ ord(chop($copy));
	}
	$mess = sprintf("\$%s*%02X",$mess,$csum);
}

sub readLat
{
	my($lat) = @_;
	# 9 6 14.04 (grad min sec.sec)
	if($lat =~ /([0-9]*)\s+([0-9]*)\s+([0-9]*\.[0-9]*)/)
	{
		$lat = $1 + ($2 +$3/60)/60;
	}
	# 9 6.234 (grad min.min)
	elsif($lat =~ /([0-9]*)\s+([0-9]*\.[0-9]*)/)
	{
		$lat = $1 + $2/60;
	}
	# 0906.234 (gradmin.min)
	elsif(index($lat,".")>3)
	{
		my($deg);
		$deg = int($lat/100);
		$lat = $deg+($lat-$deg*100)/60; 
	}
	# 9.1039 (grad.grad)
	else
	{
		$lat;
	}
}

sub readLon
{
	my($lon) = @_;
	# 9 6 14.04 (grad min sec.sec)
	if($lon =~ /([0-9]*)\s+([0-9]*)\s+([0-9]*\.[0-9]*)/)
	{
		$lon = $1 + ($2 +$3/60)/60;
	}
	# 9 6.234 (grad min.min)
	elsif($lon =~ /([0-9]*)\s+([0-9]*\.[0-9]*)/)
	{
		$lon = $1 + $2/60;
	}
	# 00906.234 (gradmin.min)
	elsif(index($lon,".")>4)
	{
		my($deg);
		$deg = int($lon/100);
		$lon = $deg+($lon-$deg*100)/60; 
	}
	# 9.1039 (grad.grad)
	else
	{
		$lon;
	}
}

sub extractTime
{
	my($point) = @_;
	my($timeStruct)=undef;
	if(defined $point->{date} && defined $point->{"time"}){
		my($year) = substr($point->{date},4,2);
	  $year +=($year<70)?2000:1900;
		$timeStruct = {
	  	year => $year,
	  	month => substr($point->{date},2,2),
	 		day => substr($point->{date},0,2),
	  	hour => substr($point->{"time"},0,2),
	  	min => substr($point->{"time"},2,2),
	  	sec => substr($point->{"time"},4,2),
		};
	}
	$timeStruct;
}

sub calcDifftime
{
	my($prev, $next) =@_;
	my($ts)={
	  	year => 0,
	  	month => 0,
	  	day => 0,
	  	hour => 0,
	  	min => 0,
	  	sec => 0,
		};
	my($secs)=undef;
	if(defined $prev && defined $next && $prev->{day}!=0 && $next->{day}!=0){
		my($prevSecs) = timegm($prev->{sec},$prev->{min},$prev->{hour},
										$prev->{day},$prev->{month}?$prev->{month}-1:0,$prev->{year});
		my($nextSecs) = timegm($next->{sec},$next->{min},$next->{hour},
										$next->{day},$next->{month}?$next->{month}-1:0,$next->{year});
		$secs = $nextSecs -$prevSecs;
		($ts->{sec},$ts->{min},$ts->{hour},$ts->{day},$ts->{month},$ts->{year}) = gmtime($secs);
	  $ts->{month}++;
	  $ts->{year}+=1900;
	}
	$ts, $secs;
}

sub trackBounds{
	my($track,$point) = @_;
	if(!defined $track->{bounds})
	{
		$track->{bounds}->{latmin}=$point->{lat};
		$track->{bounds}->{latmax}=$point->{lat};
		$track->{bounds}->{lonmin}=$point->{lon};
		$track->{bounds}->{lonmax}=$point->{lon};
	}else{
		if($point->{lat}<$track->{bounds}->{latmin}){
			$track->{bounds}->{latmin}=$point->{lat};
		}elsif($point->{lat}>$track->{bounds}->{latmax}){
			$track->{bounds}->{latmax}=$point->{lat};
		}
		if($point->{lon}<$track->{bounds}->{lonmin}){
			$track->{bounds}->{lonmin}=$point->{lon};
		}elsif($point->{lon}>$track->{bounds}->{lonmax}){
			$track->{bounds}->{lonmax}=$point->{lon};
		}
	}
}

sub analyseTrack{
	my(@tracks) = @_;
	my($prev, $next, $ix);
	my($b, $l, $a, $cog, $latMed, $d, $alt);
	my($altitude, $altitudeDown, $distance, $dog, $tourTime);
	my($SM) = 1852;
#	my(@track1) = @{$tracks[0]->{data}};
#	print "\n".$tracks[0]->{name}.", Anzahl: $#track1\n";
	my(@data,$track);
	foreach $track (@tracks){
		@data= @{$track->{data}};
		($altitude, $altitudeDown, $distance, $dog, $tourTime)=0;
#		print "\n".$track->{name}.", Anzahl: $#data\n";
		$prev = $data[0];
		$prev->{timestamp} = extractTime($prev);
		foreach $ix (0 .. $#data) {
			$next = $data[$ix];
			trackBounds($track,$next);
			$next->{timestamp} = extractTime($next);
			($next->{diffTime}) = calcDifftime($prev->{timestamp},$next->{timestamp});
			$b = ($next->{lat} - $prev->{lat})*60;
			$l = ($next->{lon} - $prev->{lon})*60;
			$latMed = ($next->{lat} + $prev->{lat})/2;
			$a = $l * cos($latMed*pi*2/360);
			$d = sqrt($a**2 + $b**2)*$SM;
			$next->{dog} = $d;
			$alt = $next->{altitude} - $prev->{altitude};
			$next->{distance} = sqrt($d**2 + $alt**2);
			if($b == 0){
				$cog = $a>=0? 90:270;
			}
			else{
				$cog = atan($a/$b)*360/(pi*2);
				$cog += $b>0? 0:180;
				$cog += $cog<0? 270:0;			 
			}
			$next->{cog} = $cog;
			$dog += $d;
			$distance += $next->{distance};
			$altitude += $alt > 0? $alt:0;
			$altitudeDown -= $alt < 0? $alt:0;
			$prev = $next;	
		}
		($tourTime) = calcDifftime($data[0]->{timestamp},$data[$#data]->{timestamp});
		$track->{altitude}=$altitude;
		$track->{altitudeDown}=$altitudeDown;
		$track->{distance}=$distance;
		$track->{dog}=$dog;
		$track->{tourTime}=$tourTime;
		if (!$bequiet) {
			print STDERR "Track: $track->{name} with ".($#data+1)." Trackpoints\n";
			print STDERR "Filename: $track->{filename}\n";
	    print STDERR "Altitide Up [m]: $altitude\n";
	    print STDERR "Altitide Down [m]: $altitudeDown\n";
	    printf STDERR "Distance over ground [km]: %.3f\n",($dog/1000);
	    printf STDERR "Distance [km]: %.3f \n",($distance/1000);
	    printf STDERR "Start time of the track: %02d.%02d.%04d %02d:%02d:%02d \n",
	    $data[0]->{timestamp}->{day},$data[0]->{timestamp}->{month},$data[0]->{timestamp}->{year},
	    $data[0]->{timestamp}->{hour},$data[0]->{timestamp}->{min},$data[0]->{timestamp}->{sec};
#	    my($hour)=($tourTime->{day}>0?$tourTime->{day}-1:0)*24+$tourTime->{hour};
	    printf STDERR "Length of time of the track: %02d:%02d:%02d \n\n",
	    	($tourTime->{day}>0?$tourTime->{day}-1:0)*24+$tourTime->{hour},$tourTime->{min},$tourTime->{sec};
#	    printf STDERR "Bounds: Lat(min)%s, Lon(min) %s, Lat(max)%s, Lon(max) %s \n\n", 
#	    	$track->{bounds}->{latmin},$track->{bounds}->{lonmin},$track->{bounds}->{latmax},$track->{bounds}->{lonmax};	    	
		}	
	}
	@tracks;
}
sub readMagellan
{
	my($wptsRef) = @_;
	my(@track);
	my($point,$wpt);
	my($ix,$side,$hem);

#	print "readMagellan Start: ".@{$wptsRef}.", ".$wptsRef." \n";

	while (<INPUT>) 
	{
		chomp;
		if ($_ eq "" || !/(\$PMGNTRK|\$PMGNWPL)/ ) {
			next;
		}
		if(/^(\$PMGNTRK)/){
			/^(?:\$PMGNTRK),([\d\.]*),([N|S]),([\d\.]*),([W|E]),(-?[\d\.]*),M,([\d\.]*),A,,([\d]*)\*.*$/;
			$hem = $2;
			$side = $4;
			$point= {lat => $1,
							 lon => $3,
							 altitude => $5,
							 "time" => $6,
							 date => $7};
			$point->{lat} = readLat($point->{lat});
			if ($hem eq "S") {
				$point->{lat} *= -1;
			}
			$point->{lon} = readLon($point->{lon});
			if ($side eq "W") {
				$point->{lon} *= -1;
			}
			validateCoordinates("exp or mag", $point, $_, "track");
			$track[$ix] = $point;
			$ix++;
		}
		elsif(/^(\$PMGNWPL)/){
			/^(?:\$PMGNWPL),([\d\.]*),([N|S]),([\d\.]*),([W|E]),(-?[\d\.]*),M,(.*),(.*),([a-z]{1,2})\*.*$/;
			$hem = $2;
			$side = $4;
			$wpt= {lat => $1,
						 lon => $3,
						 altitude => $5,
						 name => $6,
						 dsc => $7 ne ""?$7:undef,
						 symbol => $8};
			$wpt->{lat} = readLat($wpt->{lat});
			if ($hem eq "S") {
				$wpt->{lat} *= -1;
			}
			$wpt->{lon} = readLon($wpt->{lon});
			if ($side eq "W") {
				$wpt->{lon} *= -1;
			}
			validateCoordinates("exp or mag", $wpt, $_, "waypoint");
			push @{$wptsRef}, $wpt;
		}
	}
	print "readMagellan: ".@track.", ".@{$wptsRef}.", ".$wptsRef." \n";
	@track;
}

sub readNMEA
{
	my(@track);
	my($point, $oldPoint);
	my($ix,$side,$hem, $nmeaTag);
	while (<INPUT>) 
	{
		chomp;
		if ($_ eq "" || !/(\$GPGGA|\$GPRMC|\$GPGLL)/ ) {
			next;
		}
		$nmeaTag = $1;
		if($nmeaTag eq "\$GPGGA"){
			if (/^(?:\$GPGGA),([0-9\.]*),([0-9\.]*),([N|S]),([0-9\.]*),([W|E]),[0-9],[0-9]*,[0-9\.]*,([0-9\.\-]*),M,[0-9\.\-]*,[M]*,[0-9\.\-]*,.*$/) {
			$hem = $3;
			$side = $5;
			$point= {lat => $2,
							 lon => $4,
							 altitude => $6,
							 "time" => $1,
							 date => undef};
			} else {
				# print "error = $_\n";
			}
		}elsif($nmeaTag eq "\$GPRMC"){
			if (/^(?:\$GPRMC),([0-9\.]*),A,([0-9\.]*),([N|S]),([0-9\.]*),([W|E]),[0-9\.]*,[0-9\.]*,([0-9]*),[0-9]*,.*$/) {
			$hem = $3;
			$side = $5;
			$point= {lat => $2,
							 lon => $4,
							 altitude => undef,
							 "time" => $1,
							 date => $6};
			}
		}elsif($nmeaTag eq "\$GPGLL"){
			if (/^(?:\$GPGLL),([0-9\.]*),([N|S]),([0-9\.]*),([W|E]),([0-9\.]*),A.*$/) {
			$hem = $2;
			$side = $4;
			$point= {lat => $1,
							 lon => $3,
							 altitude => undef,
							 "time" => $5,
							 date => undef};
			}
		}
		$point->{lat} = readLat($point->{lat});
		if ($hem eq "S") {
			$point->{lat} *= -1;
		}
		$point->{lon} = readLon($point->{lon});
		if ($side eq "W") {
			$point->{lon} *= -1;
		}
		if($oldPoint->{lat} eq $point->{lat} && $oldPoint->{lon} eq $point->{lon}){
			if(defined($point->{altitude})){
				$oldPoint->{altitude} = $point->{altitude};
			}
			if(defined($point->{date})){
				$oldPoint->{date} = $point->{date};
			}			
		}else{
			validateCoordinates("nmea", $point, $_, "track");
			$track[$ix] = $point;
			$oldPoint = $track[$ix]; 
			$ix++;
		}
	}
	@track;
}

sub readFugawi
{
	my($wptsRef) = @_;	
	my(@track);
	my($point,$wpt);
	my($ix);
	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "" || /^(\#)/ ) {
			next;
  	}
  	if(/^(-?[\d\.]*),(-?[\d\.]*),(-?[\d\.]*),([\d]*),([\d]*).*$/){
	   	/^(-?[0-9\.]*),(-?[0-9\.]*),(-?[0-9\.]*),([0-9]*),([0-9]*).*$/;
	  	$point= {lat => $1,
	    	       lon => $2,
	      	     altitude => $3,
	        	   "time" => $5,
	          	 date => $4};    	 
	    $point->{altitude} = $point->{altitude}==-10000000.0?undef:$point->{altitude};    	 
	    $point->{date} = $point->{date}==18991230?undef:
	    	substr($point->{date},6,2) . substr($point->{date},4,2) . substr($point->{date},2,2);      	 
			validateCoordinates("fug", $point, $_, "track");
	  	$track[$ix] = $point;
			$ix++;
		}elsif(/^([^,]*),([^,]*),(-?[\d\.]*),(-?[\d\.]*),?(-?[\d\.]*)?,?([\d]*)?,?([\d]*)?.*$/){
			/^([^,]*),([^,]*),(-?[\d\.]*),(-?[\d\.]*),?(-?[\d\.]*)?,?([\d]*)?,?([\d]*)?.*$/;
			$wpt= {lat => $3,
						 lon => $4,
						 altitude => $5 ne "" && $5 >0?$5:undef,
						 name => $1,
						 dsc => $2 ne ""?$2:undef,
						 date => $6 ne "" && $6 !=18991230?substr($6,6,2) . substr($6,4,2) . substr($6,2,2):undef,
						 "time" => $7 ne ""?$7:undef};
#			print "wpt $wpt->{name}; $2; $wpt->{lat}; $wpt->{lon}; $5; $6; $7\n";
			$wpt->{lat} = readLat($wpt->{lat});
			$wpt->{lon} = readLon($wpt->{lon});
#			print "wpt $wpt->{name}; $wpt->{dsc}; $wpt->{lat}; $wpt->{lon}; $wpt->{altitude}; $6; $7\n";
			validateCoordinates("fug", $wpt, $_, "waypoint");
			push @{$wptsRef}, $wpt;
		}
	}
	@track;
}
  
sub readKompass
{
	my(@track);
	my($point);
	my($ix);
	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "") {
			next;
  	}
   	/^(-?[0-9\.]*),\s?(-?[0-9\.]*),?([0-9\.]*)?.*$/;
  	$point= {lat => $1,
    	       lon => $2,
      	     altitude => $3!=""?$3:undef,
        	   "time" => undef,
          	 date => undef};    	 
		validateCoordinates("tk or mm", $point, $_, "track");
  	$track[$ix] = $point;
		$ix++;
	}
	@track;
}  

sub readPCX5
{
	my($ifname,$joinTime,$wptsRef) =@_;
	my(@tracks);
	my($track, @data);
	my($point,$wpt);
	my($ix,$side,$hem);
	my($year,$day,$month);
	my(%monthMap)=("JAN" => "01", "FEB" => "02", "MAR" => "03", "APR" => "04", "MAY" => "05", "JUN" => "06",
	 					 		"JUL" => "07", "AUG" => "08", "SEP" => "09", "OCT" => "10", "NOV" => "11", "DEC" => "12");			
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});

	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "" || !/^(T)/ && !/^(W)/) {
			if(/^H  TN (.*)/){
#				if($1 !~ /^ACTIVE LOG d*/){
					if($track ne undef){
						my(@coord) = @data;
						$track->{data} = \@coord;
						push @tracks, $track;
						@data = ();
					}
					$track={
						filename => $filename,
						name => $1,
						path => $path,
					};
					$ix=0;
#				}
			} 
			next;
  	}
  	if(/^T/){
	   	/^T\s*([+|\-|N|S])([0-9\.]*)\s+([+|\-|W|E])([0-9\.]*)\s+([0-9]{2}-\w{3}-[0-9]{2})\s+([0-9]{2}:[0-9]{2}:[0-9]{2})\s*([\-0-9\.eE+]*).*$/;
	   	$hem = $1;
	   	$side = $3;
	  	$point= {lat => $2,
	    	       lon => $4,
	      	     altitude => $7,
	        	   "time" => $6,
	          	 date => $5};
	    $point->{lat} = readLat($point->{lat});
	    $point->{lat} *= $hem =~ /[-|S]/? -1:1;
	    $point->{lon} = readLon($point->{lon});      	     	 
	    $point->{lon} *= $side =~ /[-|W]/? -1:1;    
	# 3048000000000000000000000
	    $point->{altitude} = $point->{altitude} == 3.048E+24?undef:$point->{altitude};
	    $point->{altitude} = $point->{altitude} <= -9999?undef:$point->{altitude};
	    $point->{"time"} = substr($point->{"time"},0,2) . substr($point->{"time"},3,2) . substr($point->{"time"},6,2); 
	    ($day,$month,$year) = $point->{date} =~ /([0-9]{2})-(\w{3})-([0-9]{2})/;
	    $month = $monthMap{$month};
	    $point->{date} = $day.$month.$year;
			validateCoordinates("pcx", $point, $_, "track");
	  	$data[$ix] = $point;
			$ix++;
		}elsif(/^W/){
			/^W\s*(.{15})\s+([+|\-|N|S])([\d\.]*)\s+([+|\-|W|E])([\d\.]*)\s+([\d]{2}-\w{3}-[\d]{2})\s+([\d]{2}:[\d]{2}:[\d]{2})\s+([\-\d\.eE+]*)\s(.{40})\s+([\-\d\.eE+]*)\s+(\w*).*$/;
	   	$hem = $2;
	   	$side = $4;
			$wpt= {lat => $3,
						 lon => $5,
						 altitude => $8 ne "" && $8 >0?$8:undef,
						 name => $1,
						 dsc => $9,
						 date => $6,
						 "time" => $7,
						 symbol => $11};
			print "wpt $1;; $2; $3;; $4; $5;; $6 ;$7;; $8;; $9;; $11;\n";
			$wpt->{name} =~ s/\s+$//;			 
			$wpt->{dsc} =~ s/\s+$//;
			$wpt->{dsc} = $wpt->{dsc} ne ""?$wpt->{dsc}:undef;
			$wpt->{lat} = readLat($wpt->{lat});
	    $wpt->{lat} *= $hem =~ /[-|S]/? -1:1;
			$wpt->{lon} = readLon($wpt->{lon});
	    $wpt->{lon} *= $side =~ /[-|W]/? -1:1;    
	    $wpt->{altitude} = $wpt->{altitude} == 3.048E+24?undef:$wpt->{altitude};
	    $wpt->{"time"} = substr($wpt->{"time"},0,2) . substr($wpt->{"time"},3,2) . substr($wpt->{"time"},6,2); 
	    ($day,$month,$year) = $wpt->{date} =~ /([0-9]{2})-(\w{3})-([0-9]{2})/;
	    $month = $monthMap{$month};
	    $wpt->{date} = $day.$month.$year;
			print "wpt $wpt->{name}; $wpt->{dsc}; $wpt->{lat}; $wpt->{lon}; $wpt->{altitude}; $6; $7\n";
			validateCoordinates("pcx", $wpt, $_, "waypoint");
			push @{$wptsRef}, $wpt;
		}		
	}
	if(@data >0){
		$track->{data} = \@data;
		push @tracks, $track;
	}
	@tracks = joinTracks($joinTime,@tracks);
}  

sub readGarmin
{
	my($ifname,$joinTime) =@_;
	my(@tracks);
	my($track, @data);
	my($point);
	my($ix,$side,$hem);
	my($year,$day,$month);
#	my(%monthMap);
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});

#	while(<INPUT>)
#	{
#		if ($_ =~ /^(Track\s+ACTIVE\s+LOG).*$/)
#		{
#			last;
#		}
#	} 					 	
	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "" || !/^(Trackpoint)/ ) {
			if(/^Track\s+(.*)\s+([0-9]{2}\.[0-9]{2}\.[0-9]{4}).*/ || 
				/^Track\s+(.*)\s+([0-9]*:[0-9]*:[0-9]*).*/){
#				print "track: \"$1\"\n";
				$_ = $1;
				$_ =~ s/\s$//g;
#				print "track: \"$_\"\n";
#				if($_ !~ /^ACTIVE LOG [0-9]*/){
					if($track ne undef){
						my(@coord) = @data;
						$track->{data} = \@coord;
						push @tracks, $track;
						@data = ();
					}
					$track={
						filename => $filename,
						name => $_,
						path => $path,
					};
					$ix=0;
#				}
			} 
			next;
  	}
   	/^Trackpoint\s+([N|S])([0-9]*\s+[0-9]*\.[0-9]*)\s+([W|E])([0-9]*\s+[0-9]*\.[0-9]*)(\s+[0-9]{2}\.[0-9]{2}\.[0-9]{4})?(\s+[0-9]{2}:[0-9]{2}:[0-9]{2})?\s+([0-9]*\s+m).*$/;
   	$hem = $1;
   	$side = $3;
  	$point= {lat => $2,
    	       lon => $4,
      	     altitude => $7,
        	   "time" => $6!=""?$6:undef,
          	 date => $5!=""?$5:undef};
    $point->{lat} = readLat($point->{lat});
    $point->{lat} *= $hem =~ /[S]/? -1:1;
    $point->{lon} = readLon($point->{lon});      	     	 
    $point->{lon} *= $side =~ /[W]/? -1:1;
    if(defined $point->{"time"}){
	    $point->{"time"} =~/\s+([0-9]{2}):([0-9]{2}):([0-9]{2})/;
  #  $point->{"time"} = substr($point->{"time"},0,2) . substr($point->{"time"},3,2) . substr($point->{"time"},6,2); 
  		$point->{"time"} = $1.$2.$3;
		}
    if(defined $point->{date}){
	    ($day,$month,$year) = $point->{date} =~ /\s+([0-9]{2})\.([0-9]{2})\.([0-9]{4})/;
  	  $year = substr($year,2,2);
			$point->{date} = $day.$month.$year;
		}
#		print "d:$point->{date} t:$point->{time}\n";
		validateCoordinates("garm", $point, $_, "track");
  	$data[$ix] = $point;
		$ix++;
	}
	$track->{data} = \@data;
	push @tracks, $track;
	@tracks = joinTracks($joinTime,@tracks);
}  

sub readOverlay{
	my($ifname) =@_;
	my(@tracks);
	my($track, @data);
	my($point);
	my($ix,$lon);
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});

	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "" || !/^([X|Y])Koord.*$/ ) {
			if(/^\[Symbol\s+(.*)\]/){
				if($track ne undef){
					my(@coord) = @data;
					$track->{data} = \@coord;
					push @tracks, $track;
					@data = ();
				}
				$track={
					filename => $filename,
					name => $1,
					path => $path,
				};
				$ix=0;
			} 
			next;
  	}
   	/^XKoord[0-9]*=(-?[0-9\.]*).*$/;
   	$lon = $1;
   	$_ = <INPUT>;
   	chomp;
   	/^YKoord[0-9]*=(-?[0-9\.]*).*$/;   	
  	$point= {lat => $1,
    	       lon => $lon,
      	     altitude => undef,
        	   "time" => undef,
          	 date => undef};    	 
		validateCoordinates("ovl", $point, $_, "track");
  	$data[$ix] = $point;
		$ix++;
	}
	$track->{data} = \@data;
	push @tracks, $track;
	@tracks;
}  

sub readG7T{
	my($ifname,$joinTime) =@_;
	my(@tracks);
	my($track, @data);
	my($point);
	my($ix,$side,$hem);
	my($year,$day,$month);
	my($altitudeFactor); # 1 feet = 0.3048 m
	my(%monthMap)=("Jan" => "01", "Feb" => "02", "Mar" => "03", "Apr" => "04", "May" => "05", "Jun" => "06",
	 					 "Jul" => "07", "Aug" => "08", "Sep" => "09", "Oct" => "10", "Nov" => "11", "Dec" => "12");	
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});

	while(<INPUT>)
	{
		if ($_ =~ /^A\s+(Meters|Feet)\s+->Altitude\/Depth\s+units\s+for\s+this\s+file.*$/)
		{
			$altitudeFactor = $1 =~ /Feet/? 0.3048:1;
			last;
		}
	} 					 	
	while (<INPUT>) 
	{
		chomp;
  	if ($_ eq "" || !/^(T)/ ) {
			if(/^N  New Track Log Start - (.*),.*,.*/){
#				if($1 !~ /^ACTIVE LOG d*/){
					if($track ne undef){
						my(@coord) = @data;
						$track->{data} = \@coord;
						push @tracks, $track;
						@data = ();
					}
					$track={
						filename => $filename,
						name => $1,
						path => $path,
					};
					$ix=0;
#				}
			} 
			next;
  	}
   	/^T\s+([N|S])([0-9]*\s*[0-9]*\s*[0-9]*\.[0-9]*)\s+([W|E])([0-9]*\s*[0-9]*\s*[0-9]*\.[0-9]*)\s+\w+\s+(\w{3})\s+([0-9]{2})\s+([0-9]{2}:[0-9]{2}:[0-9]{2})\s+([0-9]{4})\s+;\s*([0-9\.e]*)?;?.*$/;

   	$hem = $1;
   	$side = $3;
   	$month = $monthMap{$5};
   	$day = $6;
   	$year = substr($8,2,2); 
  	$point= {lat => $2,
    	       lon => $4,
      	     altitude => $9 eq ""?undef:$9,
        	   "time" => $7,
          	 date => $day.$month.$year};
    $point->{lat} = readLat($point->{lat});
    $point->{lat} *= $hem =~ /[S]/? -1:1;
    $point->{lon} = readLon($point->{lon});      	     	 
    $point->{lon} *= $side =~ /[W]/? -1:1;
    $point->{date} = $point->{date} eq "311289"? undef:$point->{date};
    $point->{altitude} = $point->{altitude}==1e25?undef:$point->{altitude}*$altitudeFactor; 
    $point->{"time"} = substr($point->{"time"},0,2) . substr($point->{"time"},3,2) . substr($point->{"time"},6,2); 
		validateCoordinates("g7t", $point, $_, "track");
  	$data[$ix] = $point;
		$ix++;
	}
	$track->{data} = \@data;
	push @tracks, $track;
	@tracks = joinTracks($joinTime,@tracks);
}  

sub readGpx{
	my($ifname,$joinTime, $wptsRef) =@_;
	my(@tracks);
	my($track, @data);
	my($point,$ix);
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});
	my($level)=0;
	my($line)=undef;	
	while(<INPUT>){
		chomp;
		while(length($_ =$line.$_)>0){
#			print "LINE:$_:LINE\n";
			$line=undef;
	  	if ($_ eq ""){
	  		next;
	  	}elsif($level==0 && /(\s*<trk>)(.*)/ ){
	 			$level=1;	
	 			$line=$2;
				$track={
					filename => $filename,
					path => $path
				};
				$ix=0;
			}elsif($level==0 && /^(\s*<wpt) lat=\"([\d\.\-]*)\"\s*lon=\"([\d\.\-]*)\">(.*)/){
				$level=3;
	 			$line=$4;
		  	$point= {lat => $2,
		    	       lon => $3};
			}elsif($level==1 && /^(\s*<name>)([^<]*)<\/name>(.*)/){
				$track->{name}=$2;
	 			$line=$3;
			}elsif($level==1 && /^(\s*<trkseg>)(.*)/){
				$level=2;
	 			$line=$2;
			}elsif($level==2 && /^(\s*<trkpt) lat=\"([\d\.\-]*)\"\s*lon=\"([\d\.\-]*)\">(.*)/){
				$level=3;
	 			$line=$4;
		  	$point= {lat => $2,
		    	       lon => $3};
			}elsif($level==3 && /^(\s*<ele>)([\d\.\-]*)<\/ele>(.*)/){
				$point->{altitude}=$2;
	 			$line=$3;
			}elsif($level==3 && /^(\s*<time>)\d{2}(\d{2})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z<\/time>(.*)/){
				$point->{date}=$4.$3.$2;
				$point->{time}=$5.$6.$7;
	 			$line=$8;
			}elsif($level==3 && /^(\s*<name>)([^<]*)<\/name>(.*)/){
				$point->{name}=$2;
	 			$line=$3;
			}elsif($level==3 && /^(\s*<desc>)([^<]*)<\/desc>(.*)/){
				$point->{dsc}=$2;
	 			$line=$3;
			}elsif($level==3 && /^(\s*<sym>)([^<]*)<\/sym>(.*)/){
				$point->{symbol}=$2;
	 			$line=$3;
			}elsif( ($level==0 && /^(\s*<wpt)/) ||
							(($level==1 || $level==3) && /^(\s*<name>)/) ||
							($level==2 && /^(\s*<trkpt)/) ||
							($level==3 && /^(\s*<ele>)/) ||
							($level==3 && /^(\s*<time>)/) ||
							($level==3 && /^(\s*<desc>)/) ||
							($level==3 && /^(\s*<sym>)/)){
	 			$line = $_;
	 			$_ = <INPUT>; 
	 			chomp;
	 			s /\s*//;
#	 			print $_ ."\n";
	 			$line .= $_;
#				print "MERGED LINE:$line:MLINE\n";
			}elsif($level==3 && /^(\s*<\/trkpt>)(.*)/){
				$data[$ix++]=$point;
				$level=2;
	 			$line=$2;
			}elsif($level==3 && /^(\s*<\/wpt>)(.*)/){
				push @{$wptsRef}, $point;
				$level=0;
	 			$line=$2;
			}elsif($level==2 && /^(\s*<\/trkseg>)(.*)/){
				$level=1;
	 			$line=$2;
			}elsif($level==1 && /^(\s*<\/trk>)(.*)/){
				$level=0;
	 			$line=$2;
				my(@coord) = @data;
				$track->{data} = \@coord;
				push @tracks, $track;
				@data = ();
			}
			else{
				$line=undef;
			}
			$_=undef;
		}
	}	
	@tracks = joinTracks($joinTime,@tracks);
	@tracks;
}

sub readKml{
	my($ifname, $wptsRef) =@_;
	my(@tracks);
	my($container, @data);
	my($point,$ix);
	my($filename,$path,$suffix) = fileparse($ifname,qw{\.\w*$});
	my($level)=0;
	my($line)=undef;
	my($kind)=undef;
	while(<INPUT>){
		chomp;
		while(length($_ =$line.$_)>0){
#			print "LINE L$level: $_ :LINEEND\n";
			$line=undef;
	  	if ($_ eq ""){
	  		next;
	  	}elsif($level==0 && /(\s*<Placemark>)(.*)/ ){
	 			$level=1;	
	 			$line=$2;
#	 			print "Placemark:\n";
				$container={
					filename => $filename,
					path => $path
				};
				$ix=0;
			}elsif($level==1 && /^(\s*<name>)([^<]*)<\/name>(.*)/){
				$container->{name}=$2;
#	 			print "name:\n";
	 			$line=$3;
			}elsif($level==1 && /^(\s*<description>)([^<]*)<\/description>(.*)/){
				$container->{dsc}=$2;
#	 			print "dsc:\n";
	 			$line=$3;
			}elsif($level==1 && /^(\s*<TimeStamp><when>)\d{2}(\d{2})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z<\/when><\/TimeStamp>(.*)/){
				$container->{date}=$4.$3.$2;
				$container->{time}=$5.$6.$7;
	 			$line=$8;
			}elsif($level==1 && /(\s*<LineString>)(.*)/) {
				$level=2;
	 			$line=$2;
			}elsif($level==1 && /(\s*<LinearRing>)(.*)/) {
				$level=2;
	 			$line=$2;
			}elsif($level==1 && /(\s*<Point>)(.*)/){
				$level=4;
#	 			print "POINT:\n";
	 			$line=$2;
	 		}elsif(($level==2 || $level==4) && /^(\s*<coordinates>)(.*)/){
	 			$level++;
	 			$line=$2;	
			}elsif( ($level==1 && /^(\s*<name>)/) ||
							($level==1 && /^(\s*<description>)/)){
	 			$line = $_;
	 			$_ = <INPUT>; 
	 			chomp;
	 			s /\s*//;
	 			$line .= $_;
			}elsif(($level==3 || $level==5) && /^(\s*<\/coordinates>)(.*)/){
#				$level==3? print "Level End: $level\n":$level;
				$level--;
	 			$line=$2;
	 		}elsif($level==3){ # Trackpoints	
	 			/^([\d\.\-]*),\s*([\d\.\-]*),\s*([\d\.\-]*)\s+(.*)/;
	 			$line=$4;
	 			$point={lat=>$2,
	 							lon=>$1,
	 							altitude=>$3};	 			
				$data[$ix++]=$point;
#	 			print "Point: $1| $2| $3\n";
#	 			print "line: $4\n";
	 		}elsif($level==5){ # Waypoints	
#	 			print "Waypoint: '$_'\n";
	 			/^([\d\.\-]*),\s*([\d\.\-]*),\s*([\d\.\-]*)(.*)/;
	 			$line=$4;
	 			$container->{lat}=$2;
	 			$container->{lon}=$1;
	 			$container->{altitude}=$3;
			}elsif($level==4 && /^(\s*<\/Point>)(.*)/){
				push @{$wptsRef}, $container;
				$level=1;
	 			$line=$2;
			}elsif($level==2 && (/^(\s*<\/LineString>)(.*)/ || /^(\s*<\/LinearRing>)(.*)/)){
				$level=1;
	 			$line=$2;
				my(@coord) = @data;
				$container->{data} = \@coord;
				push @tracks, $container;
				@data = ();
			}elsif($level==1 && /^(\s*<\/Placemark>)(.*)/){
				$level=0;
	 			$line=$2;
			}
			else{
				$line=undef;
			}
			$_=undef;
		}
	}	
	@tracks;
}

sub writeNMEACoordinates{
	my($point) = @_;
	my($lon,$lat,$lon_deg,$lat_deg);
	$lon_deg = int(abs($point->{lon}));
	$lat_deg = int(abs($point->{lat}));
	$lon = abs($point->{lon});
	$lat = abs($point->{lat});
	$lon = ($lon - $lon_deg) * 60.0;
	$lat = ($lat - $lat_deg) * 60.0;
	$lon = (($lon_deg * 100.0) + $lon);
	$lat = (($lat_deg * 100.0) + $lat);
	return($lat, $lon);
}

sub writeMagellanTrack{
	my(@track) = @_;
	my($point);
	my($ix);
	my($lon,$lat,$mess);
	my($oldpoint);
#	my($altitude) = 0;
	$oldpoint= $track[0];
	if($oldpoint->{"time"} != 0 && defined($oldpoint->{"time"})){ 
		$oldpoint->{"time"} = $oldpoint->{"time"}-1;
	}
	foreach $ix (0 .. $#track) 
	{
		$point = $track[$ix];
		($lat, $lon) = writeNMEACoordinates($point);
		if($point->{"time"} <= $oldpoint->{"time"}){ 
			$point->{"time"} = $oldpoint->{"time"}+1;
			if(($point->{"time"}-60)/100 == int(($point->{"time"}-60)/100)){
				$point->{"time"} += 40; 
				if(($point->{"time"}-6000)/10000 == int(($point->{"time"}-6000)/10000)){
					$point->{"time"} += 4000; 
				}
			}
		}
		if($point->{date} == 0){ $point->{date} = "010101"};
    $mess = sprintf("PMGNTRK,%08.3f,%c,%09.3f,%c,%07.1f,M,%09.2f,A,,%-.6s",
		    $lat, ord(($point->{lat} < 0.0) ? 'S' : 'N'), 
		    $lon, ord(($point->{lon} < 0.0) ? 'W' : 'E'),$point->{altitude},
		    $point->{"time"},$point->{date});
		$mess = prepareNMEAChecksum($mess);
  	printf OUT "%s\r\n",$mess;
#		$altitude +=$point->{altitude}>$oldpoint->{altitude}?$point->{altitude}-$oldpoint->{altitude}:0;
		$oldpoint= $track[$ix];
	}
	printf OUT "\$PMGNCMD,END*3D";
#	printf STDERR "Altitude (kumulierte Hoehenmeter): %d m\n",$altitude; 	
}

sub writeMagellanWpts{
	my($wptsRef) = @_;
	my($point);
	my($ix);
	my($lon,$lat,$mess);
 	print "writeMagellanWpts: ".@{$wptsRef}.", ".$wptsRef."\n";
	foreach $ix (0 .. $#$wptsRef) 
	{
		$point = $$wptsRef[$ix];
		($lat, $lon) = writeNMEACoordinates($point);
		if($point->{symbol} != /([a-z])/ || length($point->{symbol})>2){
			$point->{symbol} = "x";
		}
    $mess = sprintf("PMGNWPL,%08.3f,%c,%09.3f,%c,%07.1f,M,%s,%s,%s",
		    $lat, ord(($point->{lat} < 0.0) ? 'S' : 'N'), 
		    $lon, ord(($point->{lon} < 0.0) ? 'W' : 'E'),$point->{altitude},
		    $point->{name},$point->{dsc},$point->{symbol});
		$mess = prepareNMEAChecksum($mess);
  	printf OUT "%s\r\n",$mess;
	}
	printf OUT "\$PMGNCMD,END*3D";
}

sub writeNMEA
{
	my(@track) = @_;
	my($point);
	my($ix);
	my($lon,$lat,$mess);
	foreach $ix (0 .. $#track) 
	{
		$point = $track[$ix];
		($lat, $lon) = writeNMEACoordinates($point);
		if($point->{date} == 0){ $point->{date} = "010101"};
    $mess = sprintf("GPGGA,%010.3f,%09.4f,%c,%010.4f,%c,1,,,%07.1f,M,,M,,",
		    $point->{"time"},
		    $lat, ord(($point->{lat} < 0.0) ? 'S' : 'N'), 
		    $lon, ord(($point->{lon} < 0.0) ? 'W' : 'E'),
		    $point->{altitude});
		$mess = prepareNMEAChecksum($mess);
		printf OUT "%s\r\n", $mess;
    $mess = sprintf("GPRMC,%010.3f,A,%09.4f,%c,%010.4f,%c,0.000000,0.000000,%-.6s,,",
		    $point->{"time"},
		    $lat, ord(($point->{lat} < 0.0) ? 'S' : 'N'), 
		    $lon, ord(($point->{lon} < 0.0) ? 'W' : 'E'),
		    $point->{date});
		$mess = prepareNMEAChecksum($mess);
		printf OUT "%s\r\n", $mess;
    $mess = sprintf("GPGLL,%09.4f,%c,%010.4f,%c,%010.3f,A",
		    $lat, ord(($point->{lat} < 0.0) ? 'S' : 'N'), 
		    $lon, ord(($point->{lon} < 0.0) ? 'W' : 'E'),
		    $point->{"time"});
		$mess = prepareNMEAChecksum($mess);
		printf OUT "%s\r\n", $mess;
		
#		$GPGGA,173130.759,5210.1921,N,01137.9746,E,1,03,25.5,0.0,M,,M,,*7A
#$GPRMC,173130.759,A,5210.1921,N,01137.9746,E,0.000000,0.000000,180805,,*0E
#$GPGLL,5210.1921,N,01137.9746,E,173130.759,A*3D
	}
}

sub writeOverlay
{
	my($singleFileMode,@tracks) = @_;
	my($point);
	my($ix);
	my($track,@data,$trkname);
	my($color) = 1;
	my($trackNr) = 1;
	my($thickness) = 106;
	$ofname = destFilename($tracks[0],"ovl",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	foreach $track (@tracks){
		@data = @{$track->{data}};	
		printf OUT "[Symbol %d]\r\nTyp=3\r\nGroup=1\r\nCol=%d\r\nZoom=1\r\nSize=%d\r\nArt=1\r\n",
		           $trackNr,$color, $thickness;
		printf OUT "Punkte=%d\r\n",$#data;
		foreach $ix (0 .. $#data) {
	  	printf OUT "XKoord%d=%10.8f\r\n",$ix,$data[$ix]{lon};
	  	printf OUT "YKoord%d=%10.8f\r\n",$ix,$data[$ix]{lat};
		}
		$color=++$trackNr;
	}
	printf OUT "[Overlay]\r\nSymbols=%d\r\n[MapLage]\r\n",--$trackNr;	
}

sub writeKompass
{
	my(@track) = @_;
	my($point);
	my($ix);
	foreach $ix (0 .. $#track) {
		$point = $track[$ix];
  	printf OUT "%10.8f, ",$point->{lat};
  	printf OUT "%10.8f",$point->{lon};
  	if(defined($point->{altitude})){printf OUT ", %0.1f",$point->{altitude};}
  	print OUT "\r\n"
	}	
}

sub writeFugawiTrack
{
	my(@track) = @_;
	my($point);
	my($ix);
	printf OUT "#Längen- und Breitenangaben sind im WGS84 Datum\r\n"
						."#\r\n"
						."#Jeder Datensatz enthält die folgenden Felder: \r\n"
						."#\r\n"
						."# Geogr. Breite in Grad und Dezimalstellen (südl. Hemisphäre neg. Vorzeichen)\r\n"
						."# Geogr. Länge in Grad und Dezimalstellen (neg. Vorzeichen: westl. Greenwich)\r\n"
						."# Höhe in Metern\r\n"
						."# Datum UTC (yyymmdd)\r\n"
						."# Uhrzeit UTC (hhmmss)\r\n";
	my($year,$day,$month);					
	foreach $ix (0 .. $#track) {
		$point = $track[$ix];
		$point->{altitude} = defined($point->{altitude})?$point->{altitude}:-10000000.0;
  	printf OUT "%010.8f,",$point->{lat};
  	printf OUT "%010.8f,",$point->{lon};
  	printf OUT "%03.1f,",$point->{altitude};
  	if(defined($point->{date})){
	  	$year = substr($point->{date},4,2);
  		$year +=($year<70)?2000:1900;
  		$month = substr($point->{date},2,2);
  		$day = substr($point->{date},0,2);
  		printf OUT "%04d%02d%02d,",$year,$month,$day;
  	}else{
	 		print OUT "18991230,";
		}
  	printf OUT "%06d\r\n",$point->{"time"};
	}	
}

sub writeFugawiWpts
{
	my($wptsRef) = @_;
	my($wpt);
	my($ix);
	printf OUT "#Längen- und Breitenangaben sind im WGS84 Datum\r\n"
						."#\r\n"
						."#Jeder Datensatz enthält die folgenden Felder: \r\n"
						."#\r\n"
						."# Wagpunktname\r\n"
						."# Wegpunktbeschreibung\r\n"
						."# Geogr. Breite in Grad und Dezimalstellen (südl. Hemisphäre neg. Vorzeichen)\r\n"
						."# Geogr. Länge in Grad und Dezimalstellen (neg. Vorzeichen: westl. Greenwich)\r\n"
						."# Höhe in Metern\r\n"
						."# Datum UTC (yyymmdd)\r\n"
						."# Uhrzeit UTC (hhmmss)\r\n";
	my($year,$day,$month);					
	foreach $wpt (@{$wptsRef}) {
		$wpt->{altitude} = defined($wpt->{altitude})?$wpt->{altitude}:-10000000.0;
		printf OUT "%s,%s,",$wpt->{name},$wpt->{dsc};
  	printf OUT "%010.8f,",$wpt->{lat};
  	printf OUT "%010.8f,",$wpt->{lon};
  	printf OUT "%03.1f,",$wpt->{altitude};
  	if(defined($wpt->{date})){
	  	$year = substr($wpt->{date},4,2);
  		$year +=($year<70)?2000:1900;
  		$month = substr($wpt->{date},2,2);
  		$day = substr($wpt->{date},0,2);
  		printf OUT "%04d%02d%02d,",$year,$month,$day;
  	}else{
	 		print OUT "18991230,";
		}
  	printf OUT "%06d\r\n",$wpt->{"time"};
	}	
}

sub writeG7T
{
	my($singleFileMode,@tracks) = @_;
	my($point);
	my($ix);
	my($track,@data,$trkname);
	$ofname = destFilename($tracks[0],"g7t",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	my($trkname)=defined($track->{name})?$track->{name}:$track->{filename};
	printf OUT "Version 2:G7T\r\n".
						 "#". gmtime() ."\r\n".						 
						 "#Sinnotts Radius=6371.0 km\r\n".
						 "D WGS-84\r\n".
						 "M DMM\r\n".
						 "Z 0.000000\r\n".
						 "A Meters ->Altitude/Depth units for this file\r\n";
	my($year,$day,$month, $wday);
	my(%monthMap);
	%monthMap=("01" => "Jan", "02" => "Feb", "03" => "Mar", "04" => "Apr", "05" => "May", "06" => "Jun",
	 					 "07" => "Jul", "08" => "Aug", "09" => "Sep", "10" => "Oct", "11" => "Nov", "12" => "Dec");			
	my($lonMin,$latMin, $tempDate);
	my($lon_deg,$lat_deg);
						 
	foreach $track (@tracks){
		@data = @{$track->{data}};
		$trkname = defined($track->{name})?$track->{filename}."_".$track->{name}:$track->{filename};
		printf OUT "N  New Track Log Start - %s,Default Color,1\r\n",$trkname;
		foreach $ix (0 .. $#data) {
			$point = $data[$ix];
			$lon_deg = int(abs($point->{lon}));
			$lat_deg = int(abs($point->{lat}));
			$lonMin = abs($point->{lon});
			$latMin = abs($point->{lat});
			$lonMin = ($lonMin - $lon_deg) * 60.0;
			$latMin = ($latMin - $lat_deg) * 60.0;
			
	  	printf OUT "T  %c%02d %07.4f ", ord(($point->{lat} < 0.0) ? 'S' : 'N'), $lat_deg, $latMin;
	  	printf OUT "%c%03d %07.4f ", ord(($point->{lon} < 0.0) ? 'W' : 'E'), $lon_deg, $lonMin;
	  	
			if($point->{date} == 0){ $point->{date} = "311289"};
	  	$year = substr($point->{date},4,2);
	  	$year +=($year<70)?2000:1900;
	  	$month = substr($point->{date},2,2);
	  	$day = substr($point->{date},0,2);
	#		if($point->{"time"} == 0){ $point->{"time"} = $ix};
	  	
	  	#$wday = substr(Day_of_Week_to_Text(Day_of_Week($year, $month, $day)),0,3);
	  	$tempDate  = localtime(timelocal(0,0,0,$day,$month-1,$year));
	    $wday = ("Sun","Mon","Tue","Wed","Thu","Fri","Sat")[$tempDate->wday];
	    $month = $monthMap{$month};
	  	printf OUT "%3s %3s %02d ",$wday,$month,$day;
	  	printf OUT "%02d:%02d:%02d ",substr($point->{"time"},0,2), substr($point->{"time"},2,2), substr($point->{"time"},4,2);
	  	printf OUT "%04d ;",$year;
	  	if(defined($point->{altitude})){
		  	printf OUT "%0.6f;\r\n",$point->{altitude};
		  }else{
		  	printf OUT "1e25;\r\n";
			}
		}
	}
}

sub writePCX5Tracks
{
	my($destFormat,$singleFileMode,@tracks) = @_;
	my($point);
	my($ix);
	my($track,@data,$trkname);
	my($gradRepresentation)=$destFormat eq "pcxg"?"DEG":"DM";
	$ofname = destFilename($tracks[0],$destFormat,$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	my($trkname)=defined($track->{name})?$track->{name}:$track->{filename};
	printf OUT "H  SOFTWARE NAME & VERSION\r\n".
						 "I  PCX5 2.09 generated from www.melibokus-biker.de, Date: ". gmtime() ."\r\n".
						 "\r\n".	
						 "H  R DATUM                IDX DA            DF            DX            DY            DZ\r\n".
						 "M  G WGS 84               121 +0.000000e+00 +0.000000e+00 +0.000000e+00 +0.000000e+00 +0.000000e+00\r\n".
						 "\r\n".
						 "H  COORDINATE SYSTEM\r\n".
						 "U  LAT LON $gradRepresentation\r\n".
						 "\r\n";
		my($lat_deg,$lon_deg);
		my($lon,$lat);
		my($year,$day,$month);
		my(%monthMap)=("" => "JAN", "01" => "JAN", "02" => "FEB", "03" => "MAR", "04" => "APR", "05" => "MAY", "06" => "JUN",
		 					 		"07" => "JUL", "08" => "AUG", "09" => "SEP", "10" => "OCT", "11" => "NOV", "12" => "DEC");			
	foreach $track (@tracks){
		@data = @{$track->{data}};
		$trkname = defined($track->{name})?$track->{filename}."_".$track->{name}:$track->{filename};
						 
		printf OUT "\r\n".
							 "\r\n".
							 "H  TN %s\r\n".
							 "\r\n".
							 "H  LATITUDE    LONGITUDE    DATE      TIME     ALT\r\n",$trkname;
		foreach $ix (0 .. $#data) {
			$point = $data[$ix];
			if($gradRepresentation eq "DEG"){
		  	printf OUT "T  %+011.8f ",$point->{lat};
	  		printf OUT "%+011.8f ",$point->{lon};
	  	}
	  	elsif($gradRepresentation eq "DM"){
				$lon_deg = int(abs($point->{lon}));
				$lat_deg = int(abs($point->{lat}));
				$lon = abs($point->{lon});
				$lat = abs($point->{lat});
				$lon = ($lon - $lon_deg) * 60.0;
				$lat = ($lat - $lat_deg) * 60.0;
				$lon = (($lon_deg * 100.0) + $lon);
				$lat = (($lat_deg * 100.0) + $lat);
				
	  		printf OUT "T %c%010.5f ", ord(($point->{lat} < 0.0) ? 'S' : 'N'), $lat;
	  		printf OUT "%c%011.5f ", ord(($point->{lon} < 0.0) ? 'W' : 'E'), $lon;
	  	}
	  	$year = substr($point->{date},4,2);
	  	$month = substr($point->{date},2,2);
	  	$day = substr($point->{date},0,2);
	  	$month = $monthMap{$month};
	  	printf OUT "%02d-%3s-%02d ",$day,$month,$year;
	  	printf OUT "%02d:%02d:%02d ",substr($point->{"time"},0,2), substr($point->{"time"},2,2), substr($point->{"time"},4,2);
	  	if(defined($point->{altitude})){
	  		printf OUT "%d \r\n",$point->{altitude};
	  	}else{
	  		printf OUT "-9999\r\n";#3.048E+24
	  	}
		}	
	}
}

sub writePCX5Wpts
{
	my($destFormat,$wptsRef) = @_;
	my($wpt);
	my($gradRepresentation)=$destFormat eq "pcxg"?"DEG":"DM";
	if(@{$wptsRef}==0){
		return;
	}
	$ofname = destWptsFilename($destFormat,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	printf OUT "H  SOFTWARE NAME & VERSION\r\n".
						 "I  PCX5 2.09 generated from www.melibokus-biker.de, Date: ". gmtime() ."\r\n".
						 "\r\n".	
						 "H  R DATUM                IDX DA            DF            DX            DY            DZ\r\n".
						 "M  G WGS 84               121 +0.000000e+00 +0.000000e+00 +0.000000e+00 +0.000000e+00 +0.000000e+00\r\n".
						 "\r\n".
						 "H  COORDINATE SYSTEM\r\n".
						 "U  LAT LON $gradRepresentation\r\n".
						 "\r\n"
						."H  IDNT            LATITUDE    LONGITUDE    DATE      TIME     ALT   DESCRIPTION                              PROXIMITY     SYMBOL ;waypts\r\n";
		my($lat_deg,$lon_deg);
		my($lon,$lat);
		my($year,$day,$month);
		my(%monthMap)=("" => "JAN", "01" => "JAN", "02" => "FEB", "03" => "MAR", "04" => "APR", "05" => "MAY", "06" => "JUN",
		 					 		"07" => "JUL", "08" => "AUG", "09" => "SEP", "10" => "OCT", "11" => "NOV", "12" => "DEC");			
	foreach $wpt (@{$wptsRef}){
		printf OUT "W  %-15s ",substr($wpt->{name},0,15);
		if($gradRepresentation eq "DEG"){
	  	printf OUT "%+011.7f ",$wpt->{lat};
  		printf OUT "%+011.7f ",$wpt->{lon};
  	}
  	elsif($gradRepresentation eq "DM"){
			$lon_deg = int(abs($wpt->{lon}));
			$lat_deg = int(abs($wpt->{lat}));
			$lon = abs($wpt->{lon});
			$lat = abs($wpt->{lat});
			$lon = ($lon - $lon_deg) * 60.0;
			$lat = ($lat - $lat_deg) * 60.0;
			$lon = (($lon_deg * 100.0) + $lon);
			$lat = (($lat_deg * 100.0) + $lat);
			
  		printf OUT "%c%010.5f ", ord(($wpt->{lat} < 0.0) ? 'S' : 'N'), $lat;
  		printf OUT "%c%011.5f ", ord(($wpt->{lon} < 0.0) ? 'W' : 'E'), $lon;
  	}
  	$year = substr($wpt->{date},4,2);
  	$month = substr($wpt->{date},2,2);
  	$day = substr($wpt->{date},0,2);
  	$month = $monthMap{$month};
  	printf OUT "%02d-%3s-%02d ",$day,$month,$year;
  	printf OUT "%02d:%02d:%02d ",substr($wpt->{"time"},0,2), substr($wpt->{"time"},2,2), substr($wpt->{"time"},4,2);
  	if(defined($wpt->{altitude})){
  		printf OUT "%5d ",$wpt->{altitude};
  	}else{
  		printf OUT "-9999 ";#3.048E+24
  	}
  	printf OUT "%-40s 0.00000e+00   %-6s\r\n",substr($wpt->{dsc},0,40),substr($wpt->{symbol},0,6);
	}
}

sub writeGarmin
{
	my($singleFileMode,@tracks) = @_;
	my($ix);
	my($point,$prevPoint,$tm,$prevTimeStruct);
	my($year,$day,$month,$hour,$min,$sec);					
	my($diffTime,$speed);
	my($lonMin,$latMin,$lon_deg,$lat_deg);
	my($track,@data,$trkname);
	$track=$tracks[0];
	$ofname = destFilename($track,"garm",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	printf OUT "Grid\tBreite/Länge hddd°mm.mmm'\r\n"
						."Datum\tWGS 84\r\n"
						."\r\n"
						."Header\tName\tStart Time\tElapsed Time\tLength\tAverage Speed\r\n";
						
	foreach $track (@tracks){
		@data = @{$track->{data}};
		$prevPoint = $data[0];
		$tm = $prevPoint->{timestamp};
		$trkname = defined($track->{name})?$track->{filename}."_".$track->{name}:$track->{filename};
		$track->{tourTime}->{hour}+=$track->{tourTime}->{day}>0?($track->{tourTime}->{day}-1)*24:0;
		$diffTime = ($track->{tourTime}->{hour}+($track->{tourTime}->{min}+$track->{tourTime}->{sec}/60)/60);
	  $speed = $diffTime?$track->{dog}/1000/$diffTime:0;
						
		printf OUT "\r\n"
							."Track\t%s\t", $trkname;
		if($tm->{day} ==0){
			printf OUT "\t";
		}
		else{
			printf OUT "%02d.%02d.%04d %02d:%02d:%02d\t", $tm->{day}, $tm->{month}, $tm->{year},
									$tm->{hour}, $tm->{min}, $tm->{sec};
		}	
		printf OUT "%02d:%02d:%02d\t%.1f km\t%.1f kph\r\n"
							."\r\n"
							."Header\tPosition\tTime\tAltitude\tDepth\tLeg Length\tLeg Time\tLeg Speed\tLeg Course\r\n"
							."\r\n",$track->{tourTime}->{hour},$track->{tourTime}->{min}, $track->{tourTime}->{sec},
							($track->{dog}/1000), $speed;
		foreach $ix (0 .. $#data) {
			$point = $data[$ix];
			$lon_deg = int(abs($point->{lon}));
			$lat_deg = int(abs($point->{lat}));
			$lonMin = abs($point->{lon});
			$latMin = abs($point->{lat});
			$lonMin = ($lonMin - $lon_deg) * 60.0;
			$latMin = ($latMin - $lat_deg) * 60.0;
			
	  	printf OUT "Trackpoint\t%c%02d %06.3f ", ord(($point->{lat} < 0.0) ? 'S' : 'N'), $lat_deg, $latMin;
	  	printf OUT "%c%02d %06.3f\t", ord(($point->{lon} < 0.0) ? 'W' : 'E'), $lon_deg, $lonMin;
			$tm = $point->{timestamp}; #extractTime($point);
			if($tm->{day}==0){
		  	printf OUT "\t";
			}else{	
		  	printf OUT "%02d.%02d.%04d ",$tm->{day},$tm->{month},$tm->{year};
		  	printf OUT "%02d:%02d:%02d\t",$tm->{hour},$tm->{min},$tm->{sec};
		  }
		  printf OUT "%d m\t\t%.1f m\t",$point->{altitude}, $point->{dog};
			if($tm->{day}==0){
		  	printf OUT "\t\t";
			}else{
		  	$prevTimeStruct = $point->{diffTime};
		  	$diffTime = ($prevTimeStruct->{hour}+($prevTimeStruct->{min}+$prevTimeStruct->{sec}/60)/60);
		  	$speed = $diffTime?$point->{dog}/1000/$diffTime:0.0;
		  	printf OUT "%02d:%02d:%02d\t%.1f kph\t",
		 							$prevTimeStruct->{hour},$prevTimeStruct->{min},$prevTimeStruct->{sec},
		 							$speed;
		 	}
	  	printf OUT "%d° true\r\n",$point->{cog};
	  	$prevPoint = $point;
		}	
	}
}

sub writeKml
{
	my($singleFileMode,$tracksRef, $wptsRef) = @_;
	my($point,$wpt);
	my($ix);
	my($color)=0;
	my(@colorMap)=(0xFF0000,0x00ff00,0x0000ff,0x000080,0xFF00FF,0x800000,0xFFFF00,0x00FFFF,0xFFFFFF,
								 0x008000,0xFF0080,0xFF8000,0x00FF80,0x8000FF,0x80FF00,0x0080FF,0x008080,0x000000,
								 0x008080,0x800080,0x808000);
	my($track,@data,$trkname);
	$ofname = destFilename($tracksRef->[0],"kml",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	printf OUT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
						."<kml xmlns=\"http://earth.google.com/kml/2.0\">\r\n"
						."<Document xmlns=\"http://earth.google.com/kml/2.0\">\r\n"
						."\t<name>gpsconv.pl</name><visibility>1</visibility>\r\n";
#						."\t<Style id=\"waypoint\">\r\n"
#						."\t\t<icon><href>root://icons/bitmap-4.png?x=32&amp;y=160\&amp;w=32&amp;h=32</href></icon>\r\n"
#						."\t</Style>\r\n"
#						."\t<Style id=\"route\">\r\n"
#						."\t\t<icon><href>root://icons/bitmap-4.png?x=160&amp;y=0\&amp;w=32&amp;h=32</href></icon>\r\n"
#						."\t</Style>\r\n"
	$color = 0;
	if(defined $wptsRef && @{$wptsRef} !=0){
		printf OUT "\t<Folder>\r\n"
						."\t\t<name>Waypoints</name>\r\n"
						."\t\t<visibility>1</visibility>\r\n";
		foreach $wpt (@{$wptsRef}){
			printf OUT "\t\t<Placemark>\r\n"
								."\t\t\t<name>%s</name>\r\n",$wpt->{name};
			$wpt->{timestamp} = extractTime($wpt);
			if($wpt->{timestamp}->{day}!=0){
				printf OUT "\t\t\t<TimeStamp><when>%04d-%02d-%02dT%02d:%02d:%02dZ</when></TimeStamp>\r\n",
				$wpt->{timestamp}->{year},$wpt->{timestamp}->{month},$wpt->{timestamp}->{day},
				$wpt->{timestamp}->{hour},$wpt->{timestamp}->{min},$wpt->{timestamp}->{sec};
			}
#   			     <time>2005-05-01T20:45:46Z</time>
			if(defined $wpt->{dsc}){
				printf OUT "\t\t\t<description>%s</description>\r\n",$wpt->{dsc};
			}
			printf OUT "\t\t\t<Point>\r\n"
								."\t\t\t\t<coordinates>%f, %f, %d</coordinates>\r\n"
								."\t\t\t</Point>\r\n"
								."\t\t</Placemark>\r\n",$wpt->{lon}, $wpt->{lat}, $wpt->{altitude};
		}
		printf OUT "\t</Folder>\r\n";
	}
	if(defined $tracksRef && @{$tracksRef}!=0){
		printf OUT "\t<Folder>\r\n"
						."\t\t<name>Tracks</name>\r\n"
						."\t\t<visibility>1</visibility>\r\n";
		foreach $track (@{$tracksRef}){
			@data = @{$track->{data}};
			$trkname = defined($track->{name})?$track->{filename}."_".$track->{name}:$track->{filename};
			printf OUT "\t\t<Placemark>\r\n"
								."\t\t\t<name>%s</name>\r\n"
								."\t\t\t<Style><LineStyle><color>FF%06x</color><width>2</width></LineStyle></Style>\r\n"
								."\t\t\t<LineString>\r\n"
								."\t\t\t\t<coordinates>\r\n", $trkname, $colorMap[$color++];
			$color%=22;
			foreach $ix (0 .. $#data) {
				$point = $data[$ix];
		  	printf OUT "%f,",$point->{lon};
		  	printf OUT "%f,",$point->{lat};
		  	printf OUT "%d ",$point->{altitude};
			}	
			printf OUT "\r\n\t\t\t\t</coordinates>\r\n"
								."\t\t\t</LineString>\r\n"
								."\t\t</Placemark>\r\n";
		}
		printf OUT "\t</Folder>\r\n";
	}
	printf OUT "</Document>\r\n"
						."</kml>";
	close OUT;
}

sub writeKmlr
{
	my($singleFileMode,$tracksRef,$wptsRef) = @_;
	$ofname = destFilename($tracksRef->[0],"kml",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	if(defined $tracksRef && @{$tracksRef}!=0){
		foreach my $track (@{$tracksRef}){
			my @data = @{$track->{data}};
			foreach my $ix (0 .. $#data) {
				my $point = $data[$ix];
		  	printf OUT "%f,",$point->{lon};
		  	printf OUT "%f,",$point->{lat};
		  	printf OUT "%d",$point->{altitude};
			printf OUT "\n";
			}	
		}
	}
	close OUT;
}

sub writeCSV
{
	my($singleFileMode,$tracksRef,$wptsRef) = @_;
	$ofname = destFilename($tracksRef->[0],"csv",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	if(defined $tracksRef && @{$tracksRef}!=0){
		foreach my $track (@{$tracksRef}){
			my @data = @{$track->{data}};
			foreach my $ix (0 .. $#data) {
				my $point = $data[$ix];
		  	printf OUT "%f,",$point->{lon};
		  	printf OUT "%f,",$point->{lat};
		  	printf OUT "%d,",$point->{altitude};
		  	printf OUT "%d",$point->{time};
			printf OUT "\n";
			}	
		}
	}
	close OUT;
}

sub writeGpx
{
	my($singleFileMode,$tracksRef,$wptsRef) = @_;
	my($point)=undef;
	my($ix,$wpt);
	my($track,@data,$trkname);
	$ofname = destFilename($$tracksRef[0],"gpx",$singleFileMode,$destFilename);
	unless (open(OUT,">$ofname")) {
		die "Cannot open '$ofname' for writing";
	}
	binmode(OUT);
	my($bounds);
	foreach $track (@{$tracksRef}){
		if(!defined $bounds)
		{
			$bounds->{latmin}=$track->{bounds}->{latmin};
			$bounds->{latmax}=$track->{bounds}->{latmax};
			$bounds->{lonmin}=$track->{bounds}->{lonmin};
			$bounds->{lonmax}=$track->{bounds}->{lonmax};
		}else{
			if($bounds->{latmin}>$track->{bounds}->{latmin}){
				$bounds->{latmin}=$track->{bounds}->{latmin};
			}elsif($bounds->{latmax}<$track->{bounds}->{latmax}){
				$bounds->{latmax}=$track->{bounds}->{latmax};
			}
			if($bounds->{lonmin}>$track->{bounds}->{lonmin}){
				$bounds->{lonmin}=$track->{bounds}->{lonmin};
			}elsif($bounds->{lonmax}<$track->{bounds}->{lonmax}){
				$bounds->{lonmax}=$track->{bounds}->{lonmax};
			}
		}		
	}
		printf OUT "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\r\n"
						."<gpx xmlns=\"http://www.topografix.com/GPX/1/1\" creator=\"gpsconv.pl $version\" version=\"1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd\">\r\n"
						."\r\n"
 						."\t<metadata>\r\n"
						."\t\t<link href=\"http://www.melibokus-biker.de/gps/gpsconv/\">\r\n"
      			."\t\t\t<text>gpsconv.pl</text>\r\n"
    				."\t\t</link>\r\n"
						."\t\t<time>%s</time>\r\n"
    				."\t\t<bounds minlat=\"%f\" minlon=\"%f\" maxlat=\"%f\" maxlon=\"%f\"/>\r\n"
  					."\t</metadata>\r\n",strftime("%Y-%m-%dT%H:%M:%SZ", gmtime()),
  					$bounds->{latmin},$bounds->{lonmin},$bounds->{latmax},$bounds->{lonmax};
  foreach $wpt (@{$wptsRef}){
		printf OUT "\t\<wpt lat=\"%f\" lon=\"%f\">\r\n",$wpt->{lat},$wpt->{lon};
		if(defined $wpt->{altitude}){
			printf OUT "\t\t<ele>%f</ele>\r\n",$wpt->{altitude};
		}
		$wpt->{timestamp} = extractTime($wpt);
		if($wpt->{timestamp}->{day}!=0){
			printf OUT "\t\t<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\r\n",
			$wpt->{timestamp}->{year},$wpt->{timestamp}->{month},$wpt->{timestamp}->{day},
			$wpt->{timestamp}->{hour},$wpt->{timestamp}->{min},$wpt->{timestamp}->{sec};
		}
#        <time>2005-05-01T20:45:46Z</time>
		printf OUT "\t\t<name>%s</name>\r\n",$wpt->{name};
		if(defined $wpt->{dsc}){
			printf OUT "\t\t<desc>%s</desc>\r\n",$wpt->{dsc};
		}
		if(defined $wpt->{symbol}){
			printf OUT "\t\t<sym>%s</sym>\r\n",$wpt->{symbol};
		}
		printf OUT "\t</wpt>\r\n";
  }
	foreach $track (@{$tracksRef}){
		@data = @{$track->{data}};
		$trkname = defined($track->{name})?$track->{filename}."_".$track->{name}:$track->{filename};
		printf OUT "\t<trk>\r\n"
							."\t\t<name>%s</name>\r\n"
							."\t\t<trkseg>\r\n", $trkname;
		foreach $ix (0 .. $#data) {
			$point = $data[$ix];
			printf OUT "\t\t\t<trkpt lat=\"%f\" lon=\"%f\">\r\n",$point->{lat},$point->{lon};
			if(defined $point->{altitude}){
				printf OUT "\t\t\t\t<ele>%f</ele>\r\n",$point->{altitude};
			}
			if($point->{timestamp}->{day}!=0){
				printf OUT "\t\t\t\t<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time>\r\n",
				$point->{timestamp}->{year},$point->{timestamp}->{month},$point->{timestamp}->{day},
				$point->{timestamp}->{hour},$point->{timestamp}->{min},$point->{timestamp}->{sec};
			}
#        <time>2005-05-01T20:45:46Z</time>
			printf OUT "\t\t\t</trkpt>\r\n";
		}	
		printf OUT "\t\t\</trkseg>\r\n"
							."\t</trk>\r\n";
	}
	printf OUT "</gpx>";
	close OUT;
}

sub validateCoordinates{
	my($srcFormat, $point, $line, $type) = @_;
	if($point->{lon} == 0 || $point->{lat} == 0){
		print STDERR "Can't proberly read this $type as '$srcFormat'!\n";
		print STDERR "check the row '$line'\n";
		close OUT;
		unlink "$ofname";
 		exit 0;
 	}
}

sub destFilename{
	my($track,$destFormat,$singleFileMode,$destFileName) = @_;
	my($ofname);
	my(%ext)=	(nmea=>".nmea",exp=>"_exp.log", mag=> "_mag.txt", ovl=>".ovl", tk=>".tk", mm=>"_mm3d.txt", 
						fug=>"_fug.txt", pcxg=>"_pcx5.trk", pcxm=>"_pcx5.trk", garm=>"_garmin.txt", g7t=>".g7t", 
						kml=>".kml", kmlr=>".kml", csv=>".csv", gpx=>".gpx");
	if(defined $destFilename){
		my($body,$path,$suffix) = fileparse($destFileName,qw{\.\w*$});
		$ofname = $path;
		if($body ne ""){
			$ofname .= $body;
			if($singleFileMode){
				if($track->{name} ne ""){
					 $ofname .= "_".$track->{name};
				} else{
					$ofname .= "_".$track->{filename};
				}				
			}
		}else{
			$ofname .= $track->{filename};
			if($singleFileMode && $track->{name} ne ""){
					 $ofname .= "_".$track->{name};
			}
		}
	}else{
		$ofname = $track->{path}.$track->{filename};
		if($singleFileMode && $track->{name} ne ""){
				 $ofname .= "_".$track->{name};
		}
	}
	$ofname .= $ext{$destFormat};
}

sub destWptsFilename{
	my($destFormat,$destFileName) = @_;
	my($ofname);
	my(%ext)=	(nmea=>".nmea",exp=>".upt", mag=> "_mag.txt", ovl=>".ovl", tk=>".tk", mm=>"_mm3d.txt", 
						fug=>"_fug.txt", pcxg=>"_pcx5.wpt", pcxm=>"_pcx5.wpt", garm=>"_garmin.txt", g7t=>".g7t", 
						kml=>".kml", kmlr=>".kml", csv=>".csv", gpx=>".gpx");
	if(defined $destFilename){
		my($body,$path,$suffix) = fileparse($destFileName,qw{\.\w*$});
		$ofname = $path;
		$ofname .= $body."_";
	}
	$ofname .="wpts";
	$ofname .= $ext{$destFormat};
}

sub joinTracks{
	my($joinTime, @tracks) = @_;
	my($ix, $diffTime);
	my(@data);
	my($prevPoint,$nextPoint);
	my($prevTrack,$nextTrack);
	my($merge)=0;
	
	if($joinTime eq 'n'){
		return @tracks;
	}
	for($ix=$#tracks;$ix>0;--$ix){
		$prevTrack = $tracks[$ix-1];
		$nextTrack = $tracks[$ix];
		if($joinTime eq 'a' && 
			$prevTrack->{name} =~ /^(ACTIVE LOG)/ && $nextTrack->{name}=~ /^(ACTIVE LOG)/){
			$merge = 1; 
		}elsif($joinTime eq 'd' &&
			$prevTrack->{name} =~ /^(ACTIVE LOG)/ && $nextTrack->{name}=~ /^(ACTIVE LOG)/){
			@data= @{$prevTrack->{data}};
			$prevPoint = $data[0];
			@data= @{$nextTrack->{data}};
			$nextPoint = $data[0];
			if($prevPoint->{date} eq $nextPoint->{date}){
				$merge=1;
			}
		}elsif($joinTime =~ /^([0-9]*$)/ &&
			$prevTrack->{name} =~ /^(ACTIVE LOG)/ && $nextTrack->{name}=~ /^(ACTIVE LOG)/){
			@data= @{$prevTrack->{data}};
			$prevPoint = $data[$#data];
			$prevPoint->{timestamp} = extractTime($prevPoint);
			@data= @{$nextTrack->{data}};
			$nextPoint = $data[0];
			$nextPoint->{timestamp} = extractTime($nextPoint);
			(undef,$diffTime) = calcDifftime($prevPoint->{timestamp},$nextPoint->{timestamp});
			if(($diffTime/60)<$joinTime){
				$merge = 1;
			}
		}
		if($merge){
			push @{$prevTrack->{data}}, @{$nextTrack->{data}};
			splice(@tracks,$ix,1);
		}
		$merge = 0;
	}
	if($joinTime eq 'd')
	{
		my($track);
		foreach $track (@tracks){
			if($track->{name} =~ /^(ACTIVE LOG)/){
				$prevPoint=@{$track->{data}}[0];
				$track->{name} = sprintf "%02d-%02d-%02d",substr($prevPoint->{date},0,2), 
				substr($prevPoint->{date},2,2),substr($prevPoint->{date},4,2);
			}
		}
	}
	@tracks;
}

sub reverseTracks{
	my(@tracks) = @_;
#	my($track);
	foreach my $track (@tracks){
		my @data;# = @{$track->{data}};
		@data = reverse(@{$track->{data}});
		$track->{data} = \@data;
	}
	@tracks;
}

sub splitTracks{
	my($tracksRef, $maxPoints) = @_;
	my(@tracks, $startIdx, $endIdx, $overlap);
  $maxPoints =~ /^([0-9\.]*),?([0-9\.]*)?$/;
  $maxPoints = $1;
	$overlap = $2; 
#	print "max:".$maxPoints.", overlap:".$overlap."\n";
	foreach my $track (@{$tracksRef}){
		my @data = @{$track->{data}};
		if($#data> $maxPoints){
			my $segNo = 1;
			for($segNo = 1; $segNo < (($#data-$maxPoints)/($maxPoints-$overlap)+2); $segNo++){
				my($trackSeg)={
							filename => $track->{filename},
							name => ($track->{name}?$track->{name}."_":"").$segNo,
							path => $track->{path},
							dsc => $track->{dsc},
							time => $track->{time},
							date => $track->{date},
						};
				$startIdx = ($segNo-1)*($maxPoints-$overlap);
				$endIdx = (($startIdx+$maxPoints)<$#data?($startIdx+$maxPoints):$#data)-1;
				my @tempData;
				for(my $i =$startIdx; $i <= $endIdx;++$i){
					 push @tempData, $data[$i];
				}
				$trackSeg->{data} = \@tempData;
				my @tempData = @{$track->{data}};
#	print "startIdx: $startIdx - $endIdx, Anzahl: $#tempData, $#data\n";
				push @tracks, $trackSeg;
			}
		} else{
			push @tracks, $track;
		}
	}
	@tracks;
}

sub help_fug{
	print STDERR "Details for the format 'fug':\n"; 
	print STDERR "This format is a text export from Fugawi for tracks and waypoints.\n"; 
	print STRERR "Tracks and waypoints are supported by gpsconv.\n";
	print STDERR "In Fugawi you have to choose the following fields for tracks\n";
	print STDERR "in exactly the same order for export:\n"; 
	print STDERR "Latitude and longitude coordinates, altitude in meters, time (UTC) and date (UTC)\n"; 
	print STDERR "In Fugawi you have to choose the following fields for waypoints\n";
	print STDERR "in exactly the same order for export:\n"; 
	print STDERR "Name, description, Latitude and longitude coordinates.\n"; 
	print STDERR "And optional altitude in meters, time (UTC) and date (UTC)\n"; 
}  

sub help_nmea{
	print STDERR "Details for the format 'nmea':\n";
	print STDERR "This format is used by same PocketPC (e.g. RoyalTec RBT 3000).\n"; 
	print STDERR "It based on the standard tags of NMEA 1803.\n"; 
	print STDERR "The standard-Tags GPGGA, GPRMC, GPGLL are used for reading and writing.\n";
}  

sub help_mag{
	print STDERR "Details for the format 'exp' and 'mag':\n"; 
	print STDERR "This format is used by the Magellan Meridian and eXplorist either on SD-Card or\n"; 
	print STDERR "internal storage. All MapSend programs can import or export the format 'mag'\n"; 
	print STRERR "Tracks and waypoints are supported by gpsconv.\n";
	print STDERR "The difference between 'mag' and 'exp' is the file extension '.txt' and '.log'\n"; 
	print STDERR "for tracks and for waypoints the file extension '.txt' for 'mag'\n";
	print STDERR "and '.upt' for 'exp'\n"; 
	print STDERR "If you want to write the track or/and waypoints to an eXplorist,\n";
	print STDERR "use the format 'exp'.\n"; 
	print STDERR "To use the track or/and waypoints with a Meridian or for importing it to MapSend,\n";
	print STDERR "use the format 'mag'.\n"; 
	print STDERR "For reading the track or waypoints with gpsconv you can use either 'exp' or 'mag',\n";
	print STDERR " both work fine.\n"; 
}  

sub help_tk_mm{
	print STDERR "Details for the format 'tk' and 'mm':\n"; 
	print STDERR "This format is used by Kompass, AlpenVerein and MagicMaps 3D.\n"; 
	print STDERR "The difference between 'tk' and 'mm' is the file extension '.tk' and '.txt'\n"; 
	print STDERR "If you want to use it with Kompass or AlpenVerein use the format 'tk'.\n"; 
	print STDERR "For use the track with MagicMaps 3D use the format 'mm'.\n"; 
	print STDERR "This format includes the position of a point (latitude and longitude)";
	print STDERR "and optional the altitude but not the timestamp!\n"; 
	print STDERR "Hint: Fugawi can read and write tracks in Kompass-Format ('tk').\n"; 
}
sub help_ovl{
	print STDERR "Details for the format 'ovl':\n"; 
	print STDERR "This format is used by Top50 and AustriaMap.\n"; 
	print STDERR "Only the text version of 'ovl' is supported by gpsconv, the binary version can't be\n";
	print STDERR "read and will not be supported.\n"; 
	print STDERR "This format includes only the position of a point (latitude and longitude) and not\n";
	print STDERR "the altitude or timestamp!\n"; 
	print STDERR "Hint: MagicMaps 3D or Kompass can read and write tracks in the Overlay-format ('ovl').\n"; 
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n 1' (separate file for each track).\n";		 
}
sub help_g7t{
	print STDERR "Details for the format 'g7t':\n"; 
	print STDERR "This format is used by G7ToWin and e.g. as upload-format for www.gps-info.de,\n";
	print STDERR "www.mtb.tourfinder.net.\n"; 
	print STRERR "At moment only tracks are supported by gpsconv.\n";
	print STDERR "Altitude in meters or feet work as well.\n";
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n 1' (separate file for each track).\n";		 
	print STDERR "Coordinates in UTM can't be read and will not be supported.\n"; 
}
sub help_pcx{
	print STDERR "Details for the format 'PCX5' ('pcx', 'pcxm', 'pcxg'):\n"; 
	print STDERR "This format is used by TTQV and GPSTrackmaker and can be read by\n";
	print STDERR "MapSource (version < 6.8) and Fugawi.\n"; 
	print STRERR "Tracks and waypoints are supported by gpsconv.\n";
  print STDERR "The differences between 'pcxm' and 'pcxg' is the representation of the geographic\n";
  print STDERR "coordinates.\n";
  print STDERR "'pcxm' use '[N|S|W|E]grad min.fraction' for the geographic ccordinates.\n";
  print STDERR "'pcxg' use '[+|-]grad.fraction' for the geographic coordinates.\n";  
	print STDERR "Only tracks can be read by Fugawi, waypoints in PCX5-Format can't be readed by Fugawi.\n";
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n 1' (separate file for each track).\n";		 
}
sub help_garm{
	print STDERR "Details for the format 'garm':\n"; 
	print STDERR "This format is a text export from Garmin MapSource.\n"; 
	print STRERR "At moment only tracks are supported by gpsconv.\n";
	print STDERR "This format can be read from GPS-Track-Analyse.\n"; 
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n n' (all tracks in one file).\n";		 
}

sub help_kml{
	print STDERR "Details for the format 'kml':\n"; 
	print STDERR "This format is a xml-based text-export from GoogleEarth.\n"; 
	print STRERR "Tracks and waypoints are supported by gpsconv.\n";
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n n' (all tracks in one file).\n";		 
}  

sub help_kmlr{
	print STDERR "Details for the format 'kmlr':\n"; 
	print STDERR "This format is a raw text-export from GoogleEarth.\n"; 
	print STDERR "This is a multi-track-format.\n";
	print STDERR "There are no proper headers and footers, use 'kml' for that.\n";
	print STDERR "Default value for -n is '-n n' (all tracks in one file).\n";		 
}  

sub help_csv{
	print STDERR "Details for the format 'csv':\n"; 
	print STDERR "This format is a simple CSV of lat,lon,alt,tim.\n"; 
	print STDERR "This is a multi-track-format.\n";
	print STDERR "Default value for -n is '-n n' (all tracks in one file).\n";		 
}  

