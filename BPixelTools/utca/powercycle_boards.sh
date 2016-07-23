# obviously this has to expand to multiple crates

fed_slots="2 4 5 6 7"
tkfec_slots=9
pxfec_slots=11

for slot in $fed_slots; do
    addr=$((0x70 + 2*$slot))
    addrs=$(printf '0x%x' $addr)
    echo ipmitool -H 192.168.1.41 -P '' -B 0 -T 0x82 -b 7 -t $addrs raw 0x30 0xff 0xde 0xad
    ipmitool -H 192.168.1.41 -P '' -B 0 -T 0x82 -b 7 -t $addrs raw 0x30 0xff 0xde 0xad
done
