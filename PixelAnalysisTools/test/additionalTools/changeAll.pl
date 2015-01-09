#!/usr/bin/perl
use strict;
use warnings;

mkdir "new" or die "Error opening new directory";

my $dac = "VcThr";
my $dac_max = 255;
my $dac_min = 0;
my $SetRelative = 10;

#open list of files
open(my $fin,  "<",  "list.dat")  or die "Can't open list.dat: $!";

while (<$fin>) {     # assigns each line in turn to $_  
    
    my $panelFileName = "";
    
    if(/(\s*)(\S+)(\s*)/){
        $panelFileName = $2;
	print "Begin $panelFileName\n";
    }
    
    open(my $in,  "<",  $panelFileName)  or die "Can't open $panelFileName: $!";
    open(my $out, ">", "new/$panelFileName") or die "Can't open $panelFileName: $!";

    while (<$in>) {     # assigns each line in turn to $_
	
	if(/($dac:)(\s+)(\d+)/){
	    my $dac_old = $3;
	    my $dac_new=$dac_old+$SetRelative;
	    
	    #CHECK NEW DAC IS IN ALLOWED RANGE
	    if($dac_new>255){
		$dac_new=255;
		print "***REACHED MAX***\n";
	    }
	    if($dac_new<0){
		$dac_new=0;
		print "***REACHED MIN***\n";
	    }
	    
	    print $out "$dac:$2$dac_new\n";
	    
	} else {
	    print $out "$_";
	}
	
    }
    
    close $out or die "$out: $!";
    close $in or die "$in: $!";
}

close $fin or die "$fin: $!";
print "+++++CHANGE COMPLETE+++++\n"
