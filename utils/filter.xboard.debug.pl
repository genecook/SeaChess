#!/usr/bin/perl

while(<>) {
    #print;
    print $' if /\# BBB /;
}

print "quit\n";
