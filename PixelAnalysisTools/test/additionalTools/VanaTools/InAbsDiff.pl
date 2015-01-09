#!/usr/bin/perl
use strict;
use warnings;

#first plots shown to gino and danek and in jan24 presentation 
#open(my $fin, "<", "intime.dat") or die "Can't open. $!";
#open(my $fabs, "<", "absolute.dat") or die "Can't open. $!";
#open(my $fout, ">", "inabsdiff.dat") or die "Can't open. $!";

#open(my $fin, "<", "184176_intime.dat") or die "Can't open. $!";
#open(my $fabs, "<", "184176_absolute.dat") or die "Can't open. $!";
#open(my $fout, ">", "184176_inabsdiff.dat") or die "Can't open. $!";

open(my $fin, "<", "initial_intime.dat") or die "Can't open. $!";
open(my $fabs, "<", "initial_absolute.dat") or die "Can't open. $!";
open(my $fout, ">", "initial_inabsdiff.dat") or die "Can't open. $!";



my @absArray = <$fabs>;

while(<$fin>){
    if(/(ROC:)(\s)(\S*)(\s)(avgThr)(\S*)(\s)(\S*)/){
	my $roc = $3;
	my $inthr = $8;
	#print "roc: $roc, inthr: $inthr\n";

	#if($roc=~/(BPix)/){ next;}

	my $absthr = 1000;
	foreach(@absArray){
	    my $line = $_;
	    
	    if($line=~/(ROC:)(\s)(\S*)(\s)(avgThr)(\S*)(\s)(\S*)/){
		my $thisroc = $3;
		$absthr = $8;
	    }
	    if($line=~/($roc)/){
		#print "roc: $roc, absthr: $absthr\n";
		last;
	    }
	}
	
	#calculate difference
	my $diff = $inthr-$absthr;
	print $fout "$roc $diff\n";
    }
}

close $fin or die "Can't close. $!";
close $fabs or die "Can't close. $!";

