#!/bin/sh

cd handin
make
cd ..

handin/proxy log/log01 $1 8000 1.0.0.1 5.0.0.1 53 3.0.0.1 &
#handin/proxy log/log02 $1 8001 2.0.0.1 5.0.0.1 53 4.0.0.1 &
