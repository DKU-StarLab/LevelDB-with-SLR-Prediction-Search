#!/usr/bin/bash

filename="leveldb/table/block.h"
filename2="leveldb/table/block_builder.h"
if [ "true" == "$2" ]; then
	echo "==> SLR True ";
	search="SLR = false";
	replace="SLR = true";
	sed -i "s/$search/$replace/" $filename
	sed -i "s/$search/$replace/" $filename2
	
else
	echo "==> SLR False ";
	search="SLR = true";
	replace="SLR = false";
	sed -i "s/$search/$replace/" $filename
	sed -i "s/$search/$replace/" $filename2
fi 

cd /home/leveldb/leveldb/build
cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

mkdir -p /home/leveldb/dbonly
rm -rf /home/leveldb/dbonly/test5   

cd /home/leveldb

data=""
if [ "osm" == "$1" ]; then
	echo "==> Running OSM ";
	g++ run.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -lsnappy -o run -g
	./run 
else
	echo "==> Running Twitter User Data ";
	g++ runtwitter.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -lsnappy -o runtwitter -g
	./runtwitter 
	data="twitter"
fi

cd /home/leveldb/leveldb/build

./db_bench --benchmarks="readseqslr${data},readseqslr${data},readseqslr${data},readseqslr${data},readseqslr${data}" --histogram=1  --use_existing_db=1 --db=/home/leveldb/dbonly/test5



