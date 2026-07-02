#!/usr/bin/bash

NIC=$1

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <ifname>"
    exit 1
fi

ip link set $NIC down
# iw $NIC set monitor active
iw $NIC set monitor control

if [ $? -ne 0 ]; then
iw $NIC set monitor control
fi

ip link set $NIC promisc on
ip link set $NIC up

info=$(iw dev $NIC info)
echo "$info"