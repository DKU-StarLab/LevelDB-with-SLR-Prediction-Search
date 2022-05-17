#!/usr/bin/bash

cd /home/leveldb/leveldb/build
cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

rm -rf /home/leveldb/dbonly/test2 

cd /home/leveldb
g++ test.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -o test -g
./test

# g++ test2.cc -Ileveldb/include -Lleveldb/build -lleveldb -lpthread -o test2 -g


g++ test2.cc -Ileveldb/include -Ileveldb -Lleveldb/build -lleveldb -lpthread -o test2 -g
./test2   

# cd /home/leveldb/leveldb/util
# g++ coding_test.cc -I../include -I.. -I../third_party/googletest/googletest/include -L../build -lpthread -o coding_test -g
# ./coding_test
# g++ coding_test.cc -Ileveldb/include -Ileveldb -Ileveldb/third_party/googletest/googletest/include -Lleveldb/build -lpthread -o coding_test -g
# ./coding_test


