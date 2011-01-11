#!/usr/bin/env perl

open(CONF, ".config") or die "Run genconfig first.";
while (<CONF>)
{
	s/^/\$/;
	eval;
}
close CONF;

open(CONF, "tmp/happyout") or die "Should not happen !";
while (<CONF>)
{
	s/^/\$/;
	eval;
}
close CONF;

open(CONF, "tmp/pop") or die "Should not happen !";
while (<CONF>)
{
	s/^/\$/;
	eval;
}
close CONF;

my $hap_edu = 1.5 - (($edu_NOW + 10) / ($edu_NOW + 20));
my $x = ($HAP_PE * $hap_edu * $happy_PROD)/($POP+1);

if ($x > 5) {
	$x = 5 + (($x - 5)/(log($x + 1) / log(6)));
}

printf("Pop: %d\tHappy: %.2f\tEdu: %.2f\t(meaning happy p.e. %.4f)\n",
	$POP, $hap_NOW, $edu_NOW, $hap_edu);

printf("Produced: %.0f\tyields: %.4f\tNeeded now: %.4f\n",
	$happy_PROD, $x, $hap_NEED);

$y=(($HAP_AVG*$hap_NOW)+($x*$ETU))/($ETU+$HAP_AVG);

printf("New happy-level: %.4f making min. retr.%%: %.2f\n", $y, (42 - $y));
