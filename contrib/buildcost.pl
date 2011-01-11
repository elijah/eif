#!/usr/bin/env perl
#
# Copyright (C) 2002-2003 by Johan van Selst and Marc Olzheim
#
# Show the cost of building things in a specific sector
#
#alias lcost "show l b >! tmp/buildcost.sb; ldump ${1:-*} type x y eff >! tmp/buildcost.dam; dump ${1:-*} x y lcm hcm mil avail >! tmp/buildcost.stat; @/usr/local/lib/eif/buildcost.pl land"
#alias pcost "show p b >! tmp/buildcost.sb; pdump ${1:-*} type x y eff laun >! tmp/buildcost.dam; dump ${1:-*} x y lcm hcm mil avail >! tmp/buildcost.stat; @/usr/local/lib/eif/buildcost.pl plane"
#alias scost "show s b >! tmp/buildcost.sb; sdump ${1:-*} type x y eff >! tmp/buildcost.dam; dump ${1:-*} x y lcm hcm mil avail >! tmp/buildcost.stat; @/usr/local/lib/eif/buildcost.pl ship"
#

chdir "tmp" || die "No tmp/ dir";

if ($#ARGV != 0) { die "Usage: $0 <land|plane|ship>"; }

my %buildcost;

open SB, "<buildcost.sb" || die "Should not happen !";
while (<SB>)
{
#	lcm hcm avail tech for ships
#	lcm hcm guns avail for units
#	lcm hcm crew avail for planes
	if (/^(\S+)\s+(\S+ )+\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/) {
		my @arr = ($3,$4,$5,$6);
		$buildcost{$1} = \@arr;
	}
}
close SB;

my %tlcm, %thcm, %tmil, %tavail, %str;
open DAM, "<buildcost.dam" || die "Should not happen !";
if ($ARGV[0] eq "plane")
{
	while (<DAM>)
	{
		next unless /^(\d+)\s+(\S+)\s+(-?\d+)\s+(-?\d+)\s+(\d+)\s+N$/;

		if ($5 < 100) {
			my $fact = (100-$5) * 0.01;
			($lcm, $hcm, $mil, $avail) = @{$buildcost{$2}};
			$tlcm{"$3,$4"} += $fact * $lcm;
			$thcm{"$3,$4"} += $fact * $hcm;
			$tmil{"$3,$4"} += $fact * $mil;
			$tavail{"$3,$4"} += $fact * $avail;
			$str{"$3,$4"} .= " ($5% $1($2))";
		}
	}
}
else
{
	while (<DAM>)
	{
		next unless /^(\d+)\s+(\S+)\s+(-?\d+)\s+(-?\d+)\s+(\d+)$/;

		if ($5 < 100) {
			my $fact = (100-$5) * 0.01;
			if ($ARGV[0] eq "ship")
			{
				($lcm, $hcm, $avail, $unused) = @{$buildcost{$2}};
			}
			else
			{
				($lcm, $hcm, $unused, $avail) = @{$buildcost{$2}};
			}
			$tlcm{"$3,$4"} += $fact * $lcm;
			$thcm{"$3,$4"} += $fact * $hcm;
			$tavail{"$3,$4"} += $fact * $avail;
			$str{"$3,$4"} .= " ($5% $1($2))";
		}
	}
}
close DAM;

$alcm = $ahcm = $aavail = $amil = 0;
my %hlcm, %hhcm, %hmil, %avail;

open STAT, "<buildcost.stat" || die "Should not happen !";
while (<STAT>)
{
	next unless /^-?\d+\s+-?\d+\s+/;
	($x, $y, $des, $sdes, $alcm, $ahcm, $amil, $aavail) = split /\s+/;
	$hlcm{"$x,$y"} = $alcm;
	$hhcm{"$x,$y"} = $ahcm;
	$hmil{"$x,$y"} = $amil;
	$havail{"$x,$y"} = $aavail;
	$des{"$x,$y"} = $des;
	$sdes{"$x,$y"} = $sdes;
}
close STAT;

sub perc
{
	($t, $a) = @_;
	return sprintf "\x1b[%dm%d/%d\x1b[0m", $t > $a ? 31 : 32, $t, $a;
}

foreach $i (keys %str)
{
	next unless ($des{$i});

	print ucfirst($ARGV[0]) . "s in $i ($des{$i}";
	if (! ($sdes{$i} eq "_"))
	{
		print "$sdes{$i}";
	}
	print "): $str{$i}\n";
	if ($ARGV[0] eq "plane")
	{
		printf "Building costs:  %s lcm   %s hcm   %s mil   %s avail\n\n",
			perc($tlcm{$i}, $hlcm{$i}), perc($thcm{$i}, $hhcm{$i}), perc($tmil{$i}, $hmil{$i}), perc($tavail{$i}, $havail{$i});
	}
	else
	{
		printf "Building costs:  %s lcm   %s hcm   %s avail\n\n",
			perc($tlcm{$i}, $hlcm{$i}), perc($thcm{$i}, $hhcm{$i}), perc($tavail{$i}, $havail{$i});
	}
}

