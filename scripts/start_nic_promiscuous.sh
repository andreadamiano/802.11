#!/usr/bin/bash

NIC=$1

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <ifname>"
    exit 1
fi

ip link set $NIC down
iw $NIC set monitor control 
ip link set wlxc8787d94fb10 promisc on
ip link set $NIC up
