#!/bin/sh

tail +4 | awk 'BEGIN {tot=0} {tot += $3} END { print "POP=" tot}' > tmp/pop
