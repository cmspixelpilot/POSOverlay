#!/usr/bin/perl
use strict;
use warnings;

my $filein = "../0deg_iter0/0deg_iter0.csv";
#my $filein = "0deg_shift1.csv";
open(my $fin, "<", "$filein") or die "Can't open. $!";
open(my $fout, ">", "$filein.dat") or die "Can't open. $!";

while(<$fin>){

    #        1         2   3    4   5    6   7   8   9   10  11      12  13   14    
    if(/(PixelBarrel)(\s)(\S*)(\s)(\S*)(\s)(\S*)(\s)(LV)(\s)(Analog)(\s)(ch,)(\S*)/){
	my $sec = $5;
	my $lyr = $7;
	my $cur = $14;
	
	my $nrocs = 0;
	if($lyr eq "L1+2") { $nrocs = 192; }
	elsif($lyr eq "L3"){
	    if($sec eq "Sec1" || $sec eq "Sec8"){ $nrocs = 160; }
	    if($sec eq "Sec4" || $sec eq "Sec5"){ $nrocs = 128; }
	    if($sec eq "Sec2" || $sec eq "Sec3" || $sec eq "Sec6" || $sec eq "Sec7"){ $nrocs = 192; }
	}
	my $avg = $cur/$nrocs;
	print $fout "AvgCurrentPerROCBPix: $cur $avg \n" ;
    }
    #        1         2   3    4   5    6   7   8   9   10  11      12  13   14    
    elsif(/(PixelEndCap)(\s)(\S*)(\s)(\S*)(\s)(\S*)(\s)(LV)(\s)(Analog)(\s)(ch,)(\S*)/){
	my $cur = $14;
	my $nrocs = 135;
	my $avg = $cur/$nrocs;
	print $fout "AvgCurrentPerROCFPix: $cur $avg \n" ;
    }
}

close $fin or die "Can't close. $!";
close $fout or die "Can't close. $!";
