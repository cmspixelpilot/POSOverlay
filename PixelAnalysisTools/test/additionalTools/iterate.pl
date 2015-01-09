#!/usr/bin/perl
use strict;
use warnings;

#open files to print ROCs to change
open(my $outSub16, ">", "sub16.dat") or die "Can't open for sub16: $!";
open(my $outSub14, ">", "sub14.dat") or die "Can't open for sub14: $!";
open(my $outSub12, ">", "sub12.dat") or die "Can't open for sub12: $!";
open(my $outSub10, ">", "sub10.dat") or die "Can't open for sub10: $!";
open(my $outSub8, ">", "sub8.dat") or die "Can't open for sub8: $!";
open(my $outSub6, ">", "sub6.dat") or die "Can't open for sub6: $!";
open(my $outSub4, ">", "sub4.dat") or die "Can't open for sub4: $!";
open(my $outSub2, ">", "sub2.dat") or die "Can't open for sub2: $!";

#open list of ROCS
open(my $rin16,  "<",  "184328_p16_ALL_fail.dat")  or die "Can't open DeltaVcThr16: $!";
open(my $rin14,  "<",  "184233_p14_ALL_fail.dat")  or die "Can't open DeltaVcThr14: $!";
open(my $rin12,  "<",  "184229_p12_ALL_fail.dat")  or die "Can't open DeltaVcThr12: $!";
open(my $rin10,  "<",  "184227_p10_ALL_fail.dat")  or die "Can't open DeltaVcThr10: $!";
open(my $rin8,  "<",  "184325_p8_ALL_fail.dat")  or die "Can't open for DeltaVcThr8: $!";
open(my $rin6,  "<",  "184324_p6_ALL_fail.dat")  or die "Can't open for DeltaVcThr6: $!";
open(my $rin4,  "<",  "184323_p4_ALL_fail.dat")  or die "Can't open for DeltaVcThr4: $!";
open(my $rin2,  "<",  "184236_p2_ALL_fail.dat")  or die "Can't open for DeltaVcThr2: $!";
open(my $rin0,  "<",  "184217_0_ALL_fail.dat")  or die "Can't open for DeltaVcThr0: $!";

my @rocs16 = <$rin16>;
my @rocs14 = <$rin14>;
my @rocs12 = <$rin12>;
my @rocs10 = <$rin10>;
my @rocs8 = <$rin8>;
my @rocs6 = <$rin6>;
my @rocs4 = <$rin4>;
my @rocs2 = <$rin2>;
my @rocs0 = <$rin0>;


################
###FAIL @ 16###
###############
foreach(@rocs16){
    my $roc16=$_; 
    my $print16=1;

    foreach(@rocs14){
	my $roc14 = $_;
	if($roc14 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs12){
	my $roc12 = $_;
	if($roc12 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs10){
	my $roc10 = $_;
	if($roc10 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs8){
	my $roc8 = $_;
	if($roc8 eq $roc16){
	    $print16=0;
	}
    }
    
    foreach(@rocs6){
	my $roc6 = $_;
	if($roc6 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc16){
	    $print16=0;
	}
    }

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc16){
	    $print16=0;
	}
    }


    if($print16){
	print $outSub2 "$roc16";
    }
}
#roc16

################
###FAIL @ 14###
###############
foreach(@rocs14){
    my $roc14=$_; 
    my $print14=1;

    foreach(@rocs12){
	my $roc12 = $_;
	if($roc12 eq $roc14){
	    $print14=0;
	}
    }

    foreach(@rocs10){
	my $roc10 = $_;
	if($roc10 eq $roc14){
	    $print14=0;
	}
    }

    foreach(@rocs8){
	my $roc8 = $_;
	if($roc8 eq $roc14){
	    $print14=0;
	}
    }
    
    foreach(@rocs6){
	my $roc6 = $_;
	if($roc6 eq $roc14){
	    $print14=0;
	}
    }

    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc14){
	    $print14=0;
	}
    }

    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc14){
	    $print14=0;
	}
    }

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc14){
	    $print14=0;
	}
    }


    if($print14){
	print $outSub4 "$roc14";
    }
}
#roc14

################
###FAIL @ 12###
###############
foreach(@rocs12){
    my $roc12=$_; 
    my $print12=1;

    foreach(@rocs10){
	my $roc10 = $_;
	if($roc10 eq $roc12){
	    $print12=0;
	}
    }

    foreach(@rocs8){
	my $roc8 = $_;
	if($roc8 eq $roc12){
	    $print12=0;
	}
    }
    
    foreach(@rocs6){
	my $roc6 = $_;
	if($roc6 eq $roc12){
	    $print12=0;
	}
    }

    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc12){
	    $print12=0;
	}
    }

    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc12){
	    $print12=0;
	}
    }

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc12){
	    $print12=0;
	}
    }


    if($print12){
	print $outSub6 "$roc12";
    }
}
#roc12

################
###FAIL @ 10###
###############
foreach(@rocs10){
    my $roc10=$_; 
    my $print10=1;

    foreach(@rocs8){
	my $roc8 = $_;
	if($roc8 eq $roc10){
	    $print10=0;
	}
    }
    
    foreach(@rocs6){
	my $roc6 = $_;
	if($roc6 eq $roc10){
	    $print10=0;
	}
    }

    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc10){
	    $print10=0;
	}
    }

    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc10){
	    $print10=0;
	}
    }

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc10){
	    $print10=0;
	}
    }


    if($print10){
	print $outSub8 "$roc10";
    }
}
#roc10


################
###FAIL @ 8###
###############
foreach(@rocs8){
    my $roc8=$_; 
    my $print8=1;
    
    foreach(@rocs6){
	my $roc6 = $_;
	if($roc6 eq $roc8){
	    $print8=0;
	}
    }

    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc8){
	    $print8=0;
	}
    }
    

    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc8){
	    $print8=0;
	}
    }
    

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc8){
	    $print8=0;
	}
    }
    
    
    if($print8){
	print $outSub10 "$roc8";
    }
}
#roc8


################
###FAIL @ 6###
###############
foreach(@rocs6){
    my $roc6=$_; 
    my $print6=1;
    
    foreach(@rocs4){
	my $roc4 = $_;
	if($roc4 eq $roc6){
	    $print6=0;
	}
    }
    
    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc6){
	    $print6=0;
	}
    }

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc6){
	    $print6=0;
	}
    }


    if($print6){
	print $outSub12 "$roc6";
    }
}
#roc6

################
###FAIL @ 4###
###############
foreach(@rocs4){
    my $roc4=$_; 
    my $print4=1;
   
    foreach(@rocs2){
	my $roc2 = $_;
	if($roc2 eq $roc4){
	    $print4=0;
	}
    }
    
    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc4){
	    $print4=0;
	}
    }

    if($print4){
	print $outSub14 "$roc4";
    }
}
#roc4

################
###FAIL @ 2###
###############
foreach(@rocs2){
    my $roc2=$_; 
    my $print2=1;

    foreach(@rocs0){
	my $roc0 = $_;
	if($roc0 eq $roc2){
	    $print2=0;
	}
    }
    
    if($print2){
	print $outSub16 "$roc2";
    }
}
#roc2


close $outSub16 or die "outAdd16: $!";
close $outSub14 or die "outAdd14: $!";
close $outSub12 or die "outAdd12: $!";
close $outSub10 or die "outAdd10: $!";
close $outSub8 or die "outAdd8: $!";    
close $outSub6 or die "outAdd6: $!";    
close $outSub4 or die "outAdd4: $!";    
close $outSub2 or die "outAdd2: $!";    


close $rin16 or die "rin16: $!";    
close $rin14 or die "rin14: $!";    
close $rin12 or die "rin12: $!";    
close $rin10 or die "rin10: $!";    
close $rin8 or die "rin8: $!";    
close $rin6 or die "rin6: $!";    
close $rin4 or die "rin4: $!";    
close $rin2 or die "rin2: $!";    
close $rin0 or die "rin0: $!";    
