#!/bin/sh
#
# Copyright (C) 2002-2003 by Johan van Selst and Marc Olzheim
#
# alias predict "prod $2 ${3:-} | ${pkglibdir}/luse.sh $1 p; prod $2 ${3:-} | ${pkglibdir}/luse.sh $1 u"
# alias iron "predict i ${1:-*}"
# alias dust "predict d ${1:-*}"
# alias lcm "predict l ${1:-*}"
# alias hcm "predict h ${1:-*}"
# alias oil "predict o ${1:-*}"
# alias radi "predict r ${1:-*}"
#
#
if [ "$#" -lt 2 ]
then
	echo 'Missing argument(s)' >&2
	exit 1
elif [ "$#" -gt 2 ]
then
	echo 'Too many arguments' >&2
	exit 1
fi

if [ "$1" = l ] ; then prod=lcm; use=l;
elif [ "$1" = h ] ; then prod=hcm; use=h;
elif [ "$1" = d ] ; then prod=dust; use=d;
elif [ "$1" = i ] ; then prod=iron; use=i;
elif [ "$1" = o ] ; then prod=oil; use=o;
elif [ "$1" = r ] ; then prod=rad; use=r;
fi

if [ "$2" = p ] ; then
	awk "/$prod/"' {n+=$5} END {print "Produced: " n}'
elif [ "$2" = u ] ; then
	perl -ne '$n += $& if /[0-9]+'$use'/; END { print "Used:     ". $n. "\n" }'
fi
