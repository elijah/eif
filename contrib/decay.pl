#!/usr/bin/env perl
#
# Copyright (C) 2003 by Marc Olzheim
#
# alias decay "show sh c >! tmp/.decay.sshc; show sh b >! tmp/.decay.sshb; sdump * type mil civ fir eff x y >! tmp/.decay.sdump; @./decay.pl"
#
my %shipavail;
my %shipdec;
my %shipciv;
my %shipmil;
my $tmp;
my $amt;
my $firsttmp;

open(CONF, ".config") or die "Run genconfig first.";
while (<CONF>)
{
	s/^/\$/;
	eval;
}
close CONF;

chdir "tmp" || die "No tmp/ directory";

open SPB, "<.decay.sshb" || die "Argh .decay.sshb";
while (<SPB>)
{
	if (/^(\S+)\s+(\S+ )+\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\$(\d+)/) {
		$shipavail{$1} = $5;
	}
}
close SSB;

open SSC, "<.decay.sshc" || die "Argh .decay.sshc";
while (<SSC>)
{
	if (/^(\S+)\s+(\S+ )+\s+(\d+\S)\s+(\d+\S)\s+(\d+\S)/) {
		$tmp = $3;
		$firsttmp = chop($tmp);
		if ($firsttmp eq "c")
		{
			$shipciv{$1} = $tmp;
			$tmp = $4;
			$firsttmp = chop($tmp);
		}
		if ($firsttmp eq "m")
		{
			$shipmil{$1} = $tmp;
		}
	}
}
close SSC;

open SPD, "<.decay.sdump" || die "Argh .decay.sdump";
while (<SPD>)
{
#	       id     type     mil     civ     fir     eff       x         y
	if (/^(\d+)\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(-?\d+)\s+(-?\d+)$/)
	{
		if ($5 > 0)
		{
# mil ship
			$shipdec{$1} =  - (($ETU * $3 / 6)
				- ($ETU * (100 - (($3 * 100) / $shipmil{$2})))
				/ 7);
			if (0 < int($shipdec{$1}))
			{
				print STDOUT $1 . " " . $2 . " : shipdec = " . $shipdec{$1} / $shipavail{$2} . "% (if not in harbor)\n";
			}
		}
		else
		{
# civ ship
			$tmp = 0;

			if ($shipciv{$2} > 0)
			{
				$tmp = $shipciv{$2};
				$amt = $4;
			} elsif ($shipmil{$2} > 0)
			{
				$tmp = $shipmil{$2};
				$amt = $3;
			}

			$shipdec{$1} =
				(($ETU * (100 - (($amt * 100) / $tmp))) / 7)
				- (($ETU * (($4 / 2) + ($3 / 5))) / 3);

			if (0 < int($shipdec{$1}))
			{
				print STDOUT $1 . " " . $2 . " : shipdec = " . $shipdec{$1} / $shipavail{$2} . "% (if not in harbor)\n";
			}
		}
	}
}
close SPD;
