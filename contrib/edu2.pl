#!/usr/bin/env perl

open(CONF, ".config") or die "Run genconfig first.";
while (<CONF>)
{
	s/^/\$/;
	eval;
}
close CONF;

open(CONF, "tmp/eduout") or die "Should not happen !";
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

my $x = ($EDU_PE * $edu_PROD)/($POP+1);

if ($x > 5) {
	$x = 5 + (($x - 5)/(log($x - 1) / log(4)));
}

printf("Pop: %d\tEdu: %.2f\nProduced: %.0f", $POP, $edu_NOW, $edu_PROD);
printf("\tyields: %.4f\n", $x);
$y=(($EDU_AVG*$edu_NOW)+($x*$ETU))/($ETU+$EDU_AVG);
printf("New edu-level: %.4f making\n", $y);
printf("tech p.e.: %.4f", ($y - $TECH_MIN_EDU)/($y - $TECH_MIN_EDU + $TECH_LAG));
printf(", res p.e.: %.4f", ($y - $RES_MIN_EDU)/($y - $RES_MIN_EDU + $RES_LAG));
printf(", happy p.e.: %.4f\n", (1.5 - (($y + 10)/($y + 20))));
