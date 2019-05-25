#!/usr/bin/perl

use strict;
use warnings;

################################################################################
# select parts that match certain defines only, or the inverse
################################################################################

my $carvit = 0;
my @select;
my @ignore;
my @invert;
my @tokens;

for my $arg (@ARGV) {
	push @select, substr ($arg, 2) if (substr ($arg, 0, 2) eq "-D");
	push @ignore, substr ($arg, 2) if (substr ($arg, 0, 2) eq "-U");
	push @invert, substr ($arg, 2) if (substr ($arg, 0, 2) eq "-I");
	$carvit++ if (substr ($arg, 0, 2) eq "-c");
}

sub token_selected	{
	my ($t) = @_;
	for my $s (@select) {
			return 1 if ($s eq $t);
	}
	for my $i (@ignore) {
			return 1 if ($i eq $t);
	}
	return 0;
}
sub token_ignored {
	for my $i_s (@select) {
		for my $i_t (@tokens) {
			return 1 if ($i_t eq "-" . $i_s);
		}
	}
	for my $i_i (@ignore) {
		for my $i_t (@tokens) {
			return 1 if ($i_t eq "+" . $i_i);
		}
	}
	return 0;
}
sub token_invert {
	my ($s,$t) = @_;
	for my $i (@invert) {
		if ($i eq $t) {
			return $s if ($s =~ s/ifndef/ifdef/g);
			return $s if ($s =~ s/ifdef/ifndef/g);
		}
	}
	return $s;
}

my ($s_last) = "-";
sub output {
	my ($s) = @_;
	$s =~ s/[ \t]+$//g;
	print $s if (!($s =~ m/^$/ && $s_last =~ m/^$/));
	$s_last = $s;
}

while (<STDIN>) {
	chomp;
	if ($_ =~ m/\#ifndef[ \t]+([^ \t]+).*$/) {
		push @tokens, "-$1";
		output (token_invert ($_ . "\n", $1)) if ((!token_ignored () && !token_selected ($1)) ^ $carvit);
	} elsif ($_ =~ m/\#ifdef[ \t]+([^ \t]+).*$/) {
		push @tokens, "+$1";
		output (token_invert ($_ . "\n", $1)) if ((!token_ignored () && !token_selected ($1)) ^ $carvit);
	} elsif ($_ =~ m/\#endif.*$/) {
		my $t = substr (pop @tokens, 1);
		output ($_ . "\n") if ((!token_ignored () && !token_selected ($t)) ^ $carvit);
	} elsif ($_ =~ m/\#else.*$/) {
		$tokens [$#tokens] =~ tr/+-/-+/;
		output ($_ . "\n") if ((!token_ignored () && !token_selected (substr ($tokens [$#tokens], 1))) ^ $carvit);
	} else {
		output ($_ . "\n") if ((!token_ignored ()) ^ $carvit);
	}
}

################################################################################

exit 0;
