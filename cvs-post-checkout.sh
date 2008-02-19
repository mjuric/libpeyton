#!/bin/sh

/usr/bin/test -f "cvs-post-checkout.sh" || \
	(echo "Please run this script in root libpeyton distribution directory"; exit -1)

(mkdir -p lib-optimized && cd lib-optimized && ln -sf ../optimized/src/libpeyton.a) && \
(mkdir -p lib           && cd lib           && ln -sf ../debug/src/libpeyton.a)
