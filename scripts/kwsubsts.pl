#!/usr/bin/perl

use strict;
use warnings;

################################################################################

my %keywords;

open F, "< $ARGV[0]" || die ("could not open $ARGV[0] for reading\n");
while (<F>) {
	chomp;
	if ($_ =~ m/\#define[ \t]+([A-Z]+)[ \t]+\"(.*)\"$/) {
		$keywords{$1} = $2;
	}
}
close F;

while (<STDIN>) {
	my $l = $_;
	if ($l =~ m/\@\@/) {
		for my $keyword (keys %keywords) {
			$l =~ s/\@\@$keyword\@\@/$keywords{$keyword}/g;
		}
	}
	print $l;
}

################################################################################

exit 0;
