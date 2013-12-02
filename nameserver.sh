#!/bin/sh

cd handin
make || exit 1
cd ..

#handin/nameserver -r log/ns.log 7.0.0.1 53 topos/topo2/topo2.servers
#handin/nameserver log/ns.log 7.0.0.1 53 topos/topo2/topo2.servers topos/topo2/topo2.lsa
handin/nameserver -r log/ns.log 127.0.0.1 53 topos/topo2/topo2.servers topos/topo2/topo2.lsa
