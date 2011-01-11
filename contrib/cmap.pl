#!/usr/bin/perl
#
# Call this script with something like 'cmap *', assuming
#
# alias cmap "dump ${1:-*} des >! tmp/cmap.dump;
#    bmdump ${1:-*} x y des >! tmp/cmap.mdump;
#    ldump ${1:-*} x y fort >! tmp/cmap.ldump;
#    pdump * ?type=ss x y laun orb >! tmp/cmap.sat;
#    look * ?type#sb&type#sbc&type#na&type#nm >! tmp/cmap.llook;
#    llook * >> tmp/cmap.llook;
#    coast * >! tmp/cmap.coast;
#    @ /usr/local/lib/eif/cmap.pl"
#
# cmap.pl - johans, July 2002

use strict;
chdir "tmp" || die 'No  tmp  subdir';

my %view;
my ($minx, $miny, $maxx, $maxy);
my %land = (
		'agribusiness', 'a',
		'airfield', '*',
		'bank', 'b',
		'capital', 'c',
		'defense', 'd',
		'enlistment', 'e',
		'fortress', 'f',
		'gold', 'g',
		'harbor', 'h',
		'headquarters', '!',
		'heavy', 'k',
		'highway', '+',
		'library/school', 'l',
		'light', 'j',
		'mine', 'm',
		'mountain', '^',
		'nuclear', 'n',
		'oil', 'o',
		'park', 'p',
		'plains', '~',
		'radar', ')',
		'refinery', '%',
		'research', 'r',
		'shell', 'i',
		'technical', 't',
		'uranium', 'u',
		'venture', 'v',
		'warehouse', 'w',
		'wilderness', '-',
		);
my %colour = (
		'me', 37,
		'sea', '34',
		'sat', '41',
		'unknown', '32',
		0, '33',
		1, '1;31',
		2, '1;32',
		3, '1;33',
		4, '1;34',
		5, '1;35',
		6, '1;36',
		7, '1;37',
		);

open(CONFIG, '../.cmaprc');
open(DUMP, 'cmap.dump') || die;
open(MDUMP, 'cmap.mdump') || die;
open(LOOK, 'cmap.llook') || die;
open(LDUMP, 'cmap.ldump') || die;
open(COAST, 'cmap.coast') || die;
open(SAT, 'cmap.sat');
my $init;
while (<CONFIG>)
{
	my ($idx, $val) = /^\s*colour\s+(\S+)\s+(\S+)/;
	next unless $idx;
	$colour{$idx} = $val;
}
while (<DUMP>)
{
	chop;
	my @line = split / /;
	next unless /^[0-9-]+ [0-9-]/;
	my ($x, $y) = @line[0..1];
	$view{"$x:$y:0"} = $line[2];
	$view{"$x:$y:1"} = $colour{me};
}
while (<MDUMP>)
{
	chop;
	my @line = split / /;
	next unless /^[0-9-]+ [0-9-]/;
	my ($x, $y) = @line[0..1];
	$view{"$x:$y:0"} = $line[2] unless $view{"$x:$y:0"};
	$view{"$x:$y:1"} = $colour{unknown} unless $view{"$x:$y:1"};
	$view{"$x:$y:1"} = $colour{sea} if $view{"$x:$y:0"} eq '.';
	$minx = $x if $x < $minx || !defined($minx);
	$miny = $y if $y < $miny || !defined($miny);
	$maxx = $x if $x > $maxx || !defined($maxx);
	$maxy = $y if $y > $maxy || !defined($maxy);
}
print "$minx:$maxx,$miny:$maxy\n";
while (<LOOK>)
{
	my ($n, $p, $d, $x, $y) = /^(\S+).*#(\d+). (\S+).*@ (-?\d+),(-?\d+)/;
	next if ! $n;
	$view{"$x:$y:0"} = $land{$d} || '?';
	$view{"$x:$y:1"} = $colour{$p} || $colour{unknown};
}
while (<LDUMP>)
{
	my @line = split /\s+/;
	my ($x, $y) = @line[2..3];
	$view{"$x:$y:l"} = 1;
}
while (<COAST>)
{
	my ($c) = /#\s*(\d+)/;
	my ($x, $y) = / (-?\d+),(-?\d+)$/;
	next unless $c;
	$view{"$x:$y:s"} = $colour{$c};
}
while (<SAT>)
{
	my @line = split /\s+/;
	next unless $line[3] eq 'Y';
	my ($x, $y) = @line[1..2];
	$view{"$x:$y:sat"} = 3;
	for my $i ($minx .. $maxx)
	{
		for my $j ($miny .. $maxy)
		{
			$view{"$i:$j:sat"} = 1
				if ((abs($x - $i) + abs($y - $j) <= 28) &&
					(abs($y - $j) <= 14));
		}
	}
}
close CONFIG;
close DUMP;
close LOOK;
close LDUMP;
close COAST;
close SAT;

print "    ";
for (my $i = $minx; $i <= $maxx; $i++)
{
	print int(abs($i) / 10) unless ( $i < 0 && $i > -10 );
	print "-" if ( $i < 0 && $i > -10 );
}
print "\n    ";
for (my $i = $minx; $i <= $maxx; $i++) { print abs($i) % 10; }
print "\n";

for (my $j = $miny; $j <= $maxy; $j++)
{
	print " " . int(abs($j) / 10) unless ( $j < 0 && $j > -10 );
	print " -" if ( $j < 0 && $j > -10 );
	print abs($j) % 10 . " ";
	for (my $i = $minx; $i <= $maxx; $i++)
	{
		my $p = $view{"$i:$j:0"} || ' ';
		my $l = $view{"$i:$j:l"};
		my $c = $view{"$i:$j:1"};
		my $s = $view{"$i:$j:s"};
		my $sat = $view{"$i:$j:sat"};
		$p = "[${s}ms[m" if $s;
		if ($s) {
			print "[${s}ms[m";
		} elsif ($c) {
			print $l
				? "[${c};${l}m$p[m"
				: "[${c}m$p[m"
		} elsif ($sat) {
			print "[${colour{sat}}m$p[m";
		} else {
			print $p;
		}
	}
	print " " . int(abs($j) / 10) unless ( $j < 0 && $j > -10 );
	print " -" if ( $j < 0 && $j > -10 );
	print abs($j) % 10 . "\n";
}

print "    ";
for (my $i = $minx; $i <= $maxx; $i++)
{
	print int(abs($i) / 10) unless ( $i < 0 && $i > -10 );
	print "-" if ( $i < 0 && $i > -10 );
}
print "\n    ";
for (my $i = $minx; $i <= $maxx; $i++) { print abs($i) % 10; }
print "\n";
