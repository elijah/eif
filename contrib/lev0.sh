#!/bin/sh
#
# Usage: lev0.sh <type> where <type> equals "edu" or "happy"
#
awk 'BEGIN {tot=0} / '"${1}"' /{ tot += $5 } END {print "'"${1}"'_PROD=" tot}' > tmp/"${1}"out
