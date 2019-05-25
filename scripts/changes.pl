#!/usr/bin/perl
use strict;
use warnings;
use Text::Format;

my $trail = 127;
my $state = 0;
my $logln;
my $total = 0;

for my $arg (@ARGV) {
	$trail = substr ($arg, 2) if (substr ($arg, 0, 2) eq "-b");
	$total = 1 if (substr ($arg, 0, 2) eq "-t");
}

sub dolog {
	my ($l,$t) = @_;
	$t .= "." if ($t !~ m/\.$/);
	my $x = Text::Format->new ({firstIndent => 0, bodyIndent => length ($l) + 2, columns => 94});
	print $x->format ("$l: $t");
}

while (<STDIN>) {
	chomp;
	if ($_ =~ m/^--------------------------/) {
		$state = 0;
	} elsif ($state == 0 && $_ =~ m/^r([a-z0-9]+) \| [a-zA-Z0-9]+ \| ([0-9\-]+) /) {
		if ($1 > $trail) {
			$logln = "#$1 ($2)";
			$state = 1;
		}
	} elsif ($state == 1 && $total == 0 && $_ =~ m/^[ \t]*([a-zA-Z0-9].*)$/) {
		dolog ($logln, $1);
		$state = 0;
	} elsif ($state == 1 && $total == 1 && $_ =~ m/^[ \t]*(.+)$/) {
		dolog ($logln, $1);
		$state = 0;
	}
}
