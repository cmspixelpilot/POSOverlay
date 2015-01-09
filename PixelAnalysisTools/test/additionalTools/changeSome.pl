#!/usr/bin/perl
use strict;
use warnings;

mkdir "new" or die "Error opening NEW directory";
#mkdir "old" or die "Error opening OLD directory";

my $dac = "VcThr";
my $dac_max = 255;
my $dac_min = 0;
my $SetRelative =-2;
my $changeCounter = 0;

#open list of files
open(my $fin,  "<",  "list.dat")  or die "Can't open list.dat: $!";
#open list of ROCS
open(my $rin,  "<",  "sub2.dat")  or die "Can't open rlist.dat: $!";
my @rocArray = <$rin>;
my $size = @rocArray;

while (<$fin>) {     # assigns each line in turn to $_  
    
    my $panelFileName = "";
    
    if(/(\s*)(\S+)(\s*)/){
        $panelFileName = $2;
	print "Begin $panelFileName\n";
    }
    
    open(my $in,  "<",  "$panelFileName")  or die "Can't open $panelFileName: $!";
    open(my $out, ">", "new/$panelFileName") or die "Can't open $panelFileName: $!";
    
    my $ROCCheck=0;
    while (<$in>) {     # assigns each line in turn to $_
	
	my $dacLine = $_;
	
        if ($dacLine=~/(ROC:)(\s+)(\S+)(\s*)/) {
	    my $baseROC = $3;

	    foreach(@rocArray){
		if(/($baseROC)(\s)/){
		    $ROCCheck=1;
		    last;
		} else {
		    $ROCCheck=0;
		}
	    }
        }

	if(($dacLine=~/($dac:)(\s+)(\d+)/) && $ROCCheck){
            my $dac_old = $3;
            my $dac_new=$dac_old+$SetRelative;
	    $changeCounter++;
	    
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
print "+++++CHANGED $changeCounter ROCs+++++\n";
print "+++++(expected to change $size)+++++\n";   
