#!/bin/sh

awk 	'/^Edu/ { print "edu_NOW=" $2 "\nhap_NOW=" $4 }
	 /^Happiness needed is/ { print "hap_NEED=" $4 }
	' >> tmp/happyout
