#!/usr/bin/perl

use strict;
use warnings;

my ($l_max) = 0;
my ($l_min) = 1024;
my ($l_cnt) = 0;
my ($l_tot) = 0;
my ($s_max);
my ($s_min);

while (<STDIN>) {
	chomp;
	if ($_ !~ m/^\$GP/) { next; } # not GP !!
	if ($_ =~ m/^\$.+\$/) { print "err = ", $_, "\n"; next; }
	if ($_ !~ m/\*[0-9A-F]{2}$/) { print "noc = ", $_, "\n"; next; }
	my ($l) = length ($_);
	if ($l > $l_max) { $l_max = $l; $s_max = $_; print "max = ", $s_max, "\n"; };
	if ($l < $l_min) { $l_min = $l; $s_min = $_; print "min = ", $s_min, "\n"; };
	$l_tot += $l;
	$l_cnt += 1;
}

print "l_min = $l_min, l_max = $l_max, l_cnt = $l_cnt, l_tot = $l_tot\n";
print "avg = ", $l_tot / $l_cnt, "\n" if ($l_cnt > 0);
print "s_min = <<", $s_min, ">>\n";
print "s_max = <<", $s_max, ">>\n";

