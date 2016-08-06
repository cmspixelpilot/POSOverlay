# this doesn't deal with multiple crates

for slot in $@; do
    addr=$((0x70 + 2*$slot))
    addrs=$(printf '0x%x' $addr)
    echo ipmitool -H 192.168.1.41 -P '' -B 0 -T 0x82 -b 7 -t $addrs raw 0x30 0xff 0xde 0xad
    ipmitool -H 192.168.1.41 -P '' -B 0 -T 0x82 -b 7 -t $addrs raw 0x30 0xff 0xde 0xad
done
