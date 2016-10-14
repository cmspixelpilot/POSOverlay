
for x in *.dmp; do
    echo $x
    dumpBinaryFiles $x | awk '/^data/ { print $3, $4, $5, $6, $7, $8, $9, $10 }' | sort | uniq -c
done

