#!/bin/sh

cd handin
make
cd ..

handin/nameserver -r log/ns.log 5.0.0.1 53 topos/topo1/topo1.servers topos/topo1/topo1.lsa
