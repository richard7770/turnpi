#!/usr/bin/perl

#print "content-length: 1000000\r\n";
print "content-type: audio/wav\r\n";
print "\r\n";
system "arecord -qf cd -D hw:1,0 -B1000000";
