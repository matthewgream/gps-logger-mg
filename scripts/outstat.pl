#!/usr/bin/perl

print STDERR "ratio calc\n";

sub do_ratio
{
	my ($nam, $src, $dst) = @_;

	my $c = 0;
	my $t = 0;
	my $tt = 0;
	my $mi = -1; my $ma = -1;

	open S, "<$src" || die ("cannot open '$src': $!\n");
	open D, "<$dst" || die ("cannot open '$dst': $!\n");

	while (<S>) {
		my $s = $_;
		my $d = <D>;
		next if ($s == 0 || $d == 0);
		my $r = ($d/$s);
		$t += $r;
		$tt += ($r * $r);
		$mi = $r if ($mi == -1 || $r < $mi);
		$ma = $r if ($ma == -1 || $r > $ma);
		$c++;
	}

	close S;
	close D;

	print "$nam: count=$c, average=", ($t / $c), ", stddev=", "XXX" ,", minimum=$mi, maximum=$ma\n";
}

print "compression ratios ...\n";
do_ratio ("raw", "raw-c0.txt", "raw-c1.txt");
do_ratio ("nmea", "nmea-c0.txt", "nmea-c1.txt");
do_ratio ("kml", "kml-c0.txt", "kml-c1.txt");
do_ratio ("csv-t", "csv-t-c0.txt", "csv-t-c1.txt");
do_ratio ("csv-b", "csv-b-c0.txt", "csv-b-c1.txt");
print "\n";

print "format ratios ... (to nmea)\n";
do_ratio ("kml", "nmea-c0.txt", "kml-c0.txt");
do_ratio ("csv-t", "nmea-c0.txt", "csv-t-c0.txt");
do_ratio ("csv-b", "nmea-c0.txt", "csv-b-c0.txt");
print "\n";

print "format ratios ... (to nmea) (compressed)\n";
do_ratio ("kml", "nmea-c0.txt", "kml-c1.txt");
do_ratio ("csv-t", "nmea-c0.txt", "csv-t-c1.txt");
do_ratio ("csv-b", "nmea-c0.txt", "csv-b-c1.txt");
print "\n";

print "format ratios ... (csv-b to csv-t)\n";
do_ratio ("csv-t/b", "csv-t-c0.txt", "csv-b-c0.txt");
print "\n";

print "format ratios ... (csv-b to csv-t) (compressed)\n";
do_ratio ("csv-t/b", "csv-t-c1.txt", "csv-b-c1.txt");
print "\n";

