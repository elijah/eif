#!/bin/sh

awk '/^Edu/ { print "edu_NOW=" $2 }' >> tmp/eduout
