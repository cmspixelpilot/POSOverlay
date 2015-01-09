#!/usr/bin/perl
use strict;
use warnings;

open(my $fin1, "<", "../0deg_iter0/0deg_iter0_oneLiner.log") or die "Can't open. $!";
open(my $fin2, "<", "../0deg_iter19/0deg_iter19_oneLiner.log") or die "Can't open. $!";
open(my $fout, ">", "overallOut.dat") or die "Can't open. $!";
my @ain2 = <$fin2>;

while(<$fin1>){
    if(/(\S)(Pix)(\S*)(\s)(\S*)(\s)(\S*)(\s)(\S*)/){
	my $roc = "$1$2$3";
	my $vana1 = $5;
	my $thr1 = $7;
	my $delta1 = $9;


	my $vana2 = 0;
	my $thr2 = 0;
	my $delta2 = 0;
	foreach(@ain2){
	    if(/($roc)(\s)(\S*)(\s)(\S*)(\s)(\S*)/){
		$vana2 = $3;
		$thr2 = $5;
		$delta2 = $7;
		last;
	    }
	}
	
	my $vanadiff = $vana2-$vana1;
	my $thrdiff = $thr2-$thr1;
	print $fout "$roc $vanadiff $thrdiff\n";
	#print "$roc $vanadiff $thrdiff\n";
    }
}
