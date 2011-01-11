#!/bin/sh
# Copyright (C) 2003 by Marc Olzheim
#
# alias comco "dump ${1:-*} ${2:-civ} ${3:-civ} | $(pkglibdir)/comcount.sh"
#
awk 'BEGIN { n = 0; parse = 0; }
	/Unrecognized field/ { print; type = "UNKNOWN"; exit 1; }
	/^x y / { parse = 1; type = $3; }
	(parse == 1) { n = n + $3; }
	END {  print type ":" n "\n"; }'
