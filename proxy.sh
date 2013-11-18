#!/bin/sh

cd handin
make
cd ..

handin/proxy log/log01 0.2 8000 1.0.0.1 5.0.0.1 53 3.0.0.1 &
handin/proxy log/log02 0.2 8000 2.0.0.1 5.0.0.1 53 4.0.0.1 &
