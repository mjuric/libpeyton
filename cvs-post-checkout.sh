#!/bin/sh

/usr/bin/test -d "lib-optimized" || \
	(echo "Please run this script in root libpeyton distribution directory"; exit -1)

cd lib-optimized
ln -sf ../optimized/src/libpeyton.a
cd ../lib
ln -sf ../debug/src/libpeyton.a
cd ..
