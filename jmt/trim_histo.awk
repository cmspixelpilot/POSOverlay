#!/usr/bin/awk -f
{ mod = roc = $2; sub(/_PLQ.*/, "", mod); mods[mod] += 1; rocs[roc] += 1 }
END { for (x in mods) print x, mods[x]; print ""; for (x in rocs) print x, rocs[x]; }

