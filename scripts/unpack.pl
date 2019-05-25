#!/usr/bin/perl

my $_LICENSE_BEGIN_ = <<_LICENSE_END_;

	unpack.pl: gps logger utility for unpack of CSV binary encoding.
	copyright (c) 2007 gps_logger_mg@tanundra.com. all rights reserved.
	refer to the associated LICENSE file for licensing terms/conditions.

_LICENSE_END_

use strict;
use warnings;

use POSIX qw (ceil floor);
use Getopt::Long;

################################################################################

# secs since midnight, converted to hours/minutes/seconds
sub secstohhmmss {
	my ($ss) = @_;
	my ($hh) = POSIX::floor ($ss / (60 * 60)); $ss -= $hh * (60 * 60);
	my ($mm) = POSIX::floor ($ss / 60); $ss -= $mm * 60;
	return sprintf ("%02d%02d%02d", $hh, $mm, $ss);
}

# days since 1980/1/1, converted to day/month/year
sub daystoddmmyy {
	my ($xx) = @_;
	my ($t1) = (($xx + 32083 + 2444253) * 4) - 1;
	my ($t2) = (((($t1 % 1461) / 4) + 1) * 5) - 3;
	my ($dd) = (($t2 % 153) / 5) + 1;
	my ($mm) = ($t2 / 153);
	my ($yy) = (($t1 / 1461) - 4800) + (($mm < 10) ? 0 : 1);
	return sprintf ("%02d%02d%04d", $dd, ($mm < 10) ? ($mm + 3) : ($mm - 9), $yy);
}

################################################################################

my $bits_str = "";

sub bits_ini {
	binmode (STDIN, ":raw") || die ("fatal: cannot enable binmode for STDIN\n");
}

sub bits_eof {
	return (eof (STDIN) && !length ($bits_str)) ? 1 : 0;
}

sub bits_get {
	my ($n) = @_;
	if (length ($bits_str) < $n) {
			exit (0) if (eof (STDIN)); # i.e. mid-record
			my ($bufdat);
			if (read (STDIN, $bufdat, 4096) > 0) {
					$bits_str .= unpack ("B*", $bufdat);
			}
	}
	my ($s) = substr ($bits_str, 0, $n);
	$bits_str = substr ($bits_str, $n);
	return unpack ("N", pack ("B32", substr ("0" x 32 . $s, -32)));
}

################################################################################

my $cfg_elements = "";
my $cfg_debug = "";
my $cfg_newline = "";

printf STDERR "gps_logger_mg v" .q(@@VERSION@@). ": utility csv-unpack\n";
printf STDERR q(@@COPYRIGHT@@). "\n";

my $result = GetOptions ("elements=s" => \$cfg_elements,
						 "version" => sub { exit; },
						 "newline" => \$cfg_newline,
						 "help" => sub { print STDERR "usage: $0 [-v] [-h] [-d] [-n] -e <lon,lat,alt,tim,dat>\n"; exit; },
						 "debug" => \$cfg_debug);

print STDERR "elements = $cfg_elements\n" if ($cfg_debug);
print STDERR "newline = $cfg_newline\n" if ($cfg_debug);
print STDERR "debug = $cfg_debug\n" if ($cfg_debug);

$cfg_elements || die ("fatal: a comma separated list of elements must be defined\n");

my ($nnn_lat_sign, $nnn_lat_majr, $nnn_lat_minr);
my ($nnn_lon_sign, $nnn_lon_majr, $nnn_lon_minr);
my ($nnn_alt_valu);
my ($nnn_tim_valu);
my ($nnn_dat_valu);

my (%cfg_defined) = (
	"lat" => sub { 
					if (bits_get (1) == 0) {
						$nnn_lat_sign = bits_get (1);
						$nnn_lat_majr = bits_get (7);
						$nnn_lat_minr = bits_get (20);
					} else {
						my ($r_sign) = bits_get (1);
						my ($r_valu) = bits_get (8);
						$r_valu = -$r_valu if ($r_sign == 1);
						$nnn_lat_minr += $r_valu;
					}
					return ($nnn_lat_sign == 1 ? "-" : "") . $nnn_lat_majr . "." . sprintf ("%06d", $nnn_lat_minr);
			},
	"lon" => sub { 
					if (bits_get (1) == 0) {
						$nnn_lon_sign = bits_get (1);
						$nnn_lon_majr = bits_get (8);
						$nnn_lon_minr = bits_get (20);
					} else {
						my ($r_sign) = bits_get (1);
						my ($r_valu) = bits_get (9);
						$r_valu = -$r_valu if ($r_sign == 1);
						$nnn_lon_minr += $r_valu;
					}
					return ($nnn_lon_sign == 1 ? "-" : "") . $nnn_lon_majr . "." . sprintf ("%06d", $nnn_lon_minr);
			},
	"alt" => sub {
					if (bits_get (1) == 0) {
						my ($a_sign) = bits_get (1);
						$nnn_alt_valu = bits_get (15);
						$nnn_alt_valu = -$nnn_alt_valu if ($a_sign == 1);
					} else {
						my ($r_sign) = bits_get (1);
						my ($r_valu) = bits_get (1);
						$r_valu = -$r_valu if ($r_sign == 1);
						$nnn_alt_valu += $r_valu;
					}
					return $nnn_alt_valu;
			},
	"tim" => sub {
					if (bits_get (1) == 0) {
						$nnn_tim_valu = bits_get (17);
					} else {
						my ($r_valu) = bits_get (1);
						$nnn_tim_valu += $r_valu;
					}
					return secstohhmmss ($nnn_tim_valu);
			},
	"dat" => sub {
					if (bits_get (1) == 0) {
						$nnn_dat_valu = bits_get (14);
					} else {
						# nothing!
					}
					return daystoddmmyy ($nnn_dat_valu);
			}
);

################################################################################

my @rec_elements;
for my $e (split (",", $cfg_elements)) {
	exists ($cfg_defined{$e}) || die ("fatal: element <$e> is not valid\n");
	push @rec_elements, $cfg_defined{$e};
}

################################################################################

bits_ini ();
# my $rec_number = 0;
while (!bits_eof ()) {
	my (@rec_results);
	for my $rec_decode (@rec_elements) {
		push @rec_results, &$rec_decode ();
	}
	print STDOUT join (',', @rec_results), ($cfg_newline ? "\r\n" : "\n");
# 	$rec_number++;
}
# printf STDERR "completed: $rec_number records decoded\n" if ($cfg_debug);

################################################################################

exit 0;
