// #include <cassert>
// #include "leveldb/db.h"

// leveldb::DB* db;
// leveldb::Options options;
// options.create_if_missing = true;
// leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
// assert(status.ok());

#include <iostream>
#include <cassert>
#include "leveldb/db.h"
#include "leveldb/write_batch.h"

int main()
{
    // Open a database.
    leveldb::DB* db;
    leveldb::Options opts;
    opts.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(opts, "./testdb", &db);
    assert(status.ok());

    // Write data.
    status = db->Put(leveldb::WriteOptions(), "name", "jinhelin");
    assert(status.ok());

    // Read data.
    std::string val;
    status = db->Get(leveldb::ReadOptions(), "name", &val);
    assert(status.ok());
    std::cout << val << std::endl;

    // Batch atomic write.
    leveldb::WriteBatch batch;
    batch.Delete("name");
    batch.Put("name0", "jinhelin0");
    batch.Put("name1", "jinhelin1");
    batch.Put("name2", "jinhelin2");
    batch.Put("name3", "jinhelin3");
    batch.Put("name4", "jinhelin4");
    batch.Put("name5", "jinhelin5");
    batch.Put("name6", "jinhelin6");
    batch.Put("name7", "jinhelin7");
    batch.Put("name8", "jinhelin8");
    batch.Put("name9", "jinhelin9");
    status = db->Write(leveldb::WriteOptions(), &batch);
    assert(status.ok());

    // Scan database.
    leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        std::cout << it->key().ToString() << ": " << 
          it->value().ToString() << std::endl;
    }
    assert(it->status().ok());

    // Range scan, example: [name3, name8)
    for (it->Seek("name3"); 
         it->Valid() && it->key().ToString() < "name8"; 
         it->Next()) {
        std::cout << it->key().ToString() << ": " << 
          it->value().ToString() << std::endl;
    } 

    // Close a database.
    delete db;
}

