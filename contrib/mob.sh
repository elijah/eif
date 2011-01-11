#!/bin/sh
# Copyright (C) 1999 by Marc Olzheim
#
# alias ss2 "sdump $* type eff mob tech spd name | ${pkglibdir}/mob.sh"
#
# mobpsect = 480 / (($spd + $spd * (50 + $tech) / (200 + $tech)) * $eff / 100)
#
awk	'BEGIN {
		print "shp#     eff spd tech  mob mb/sc sects"
	}
	/"$/ {
		mobpsect = 480/(($6+$6*(50+$5)/(200+$5)) * $3 / 100);
		printf "%4d %-3s %3d %3d %4d % 4d %5.2f %5.2f\n", $1, $2, $3, $6, $5, $4, mobpsect, $4/mobpsect
	}
	/ships$/ {
		print
	}'
