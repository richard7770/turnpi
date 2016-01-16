#!/usr/bin/perl

#print "content-length: 1000000\r\n";
print "content-type: audio/mpeg\r\n";
print "\r\n";
system "arecord -qf cd -D hw:1,0 -B50000| lame --cbr -t -b 320 --quiet - -";

