#!/usr/bin/bash

cd /home/leveldb/leveldb/build
cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

rm -rf /home/leveldb/dbonly/test5   

cd /home/leveldb


g++ test.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -o test -g
./test 
 
# g++ test2.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -o test2 -g


g++ test2.cc -Ileveldb/include -Ileveldb -Lleveldb/build -lleveldb -lpthread -o test2 -g
./test2   

# cd /home/leveldb/leveldb/benchmarks

# g++ db_bench.cc -I../include -I/home/leveldb/leveldb -I../build -I../third_party/googletest/googlemock/include -I../third_party/googletest/googletest/include -lleveldb -lpthread -o db_bench -g
cd /home/leveldb/leveldb/build

# sync; echo 3 > /proc/sys/vm/drop_caches 
# ./db_bench --benchmarks=readseqslr --histogram=1  --use_existing_db=1 --db=/home/leveldb/dbonly/test5
./db_bench --benchmarks="readseqslr,readseqslr,readseqslr,readseqslr,readseqslr" --histogram=1  --use_existing_db=1 --db=/home/leveldb/dbonly/test5

#clearing kernel buffer before running each workload
# sync; echo 3 > /proc/sys/vm/drop_caches 

# cd /home/leveldb/leveldb/util
# g++ coding_test.cc -I../include -I.. -I../third_party/googletest/googletest/include -L../build -lpthread -o coding_test -g
# ./coding_test
# g++ coding_test.cc -Ileveldb/include -Ileveldb -Ileveldb/third_party/googletest/googletest/include -Lleveldb/build -lpthread -o coding_test -g
# ./coding_test


