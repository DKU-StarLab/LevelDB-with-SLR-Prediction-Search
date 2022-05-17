// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_H_

#include <cstddef>
#include <cstdint>

#include "leveldb/iterator.h"

namespace leveldb {

struct BlockContents;
class Comparator;

class Block {
 public:
  // Initialize the block with the specified contents.
  
  
  explicit Block(const BlockContents& contents);

  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  ~Block();

  size_t size() const { return size_; }
  Iterator* NewIterator(const Comparator* comparator);

 private:
  class Iter;

  uint32_t NumRestarts() const;
  
  //define constant of LR dividend
  uint32_t MetaPosition(int type,int pos) const;
  uint32_t Dividend(int pos) const;
  uint32_t Divisor(int pos) const;
  uint32_t Lowest(int pos) const;
  
  // vector<int> LowestLists();

  const char* data_;
  size_t size_;
  uint32_t restart_offset_;  // Offset in data_ of restart array
  bool owned_;               // Block owns data_[]
  
  // int new_meta_size = 17;  // total of meta in block builder 
  int new_meta_size = 32;  // total of meta in block builder  

  bool SLR = true;			// set to false to use default binary search         
  bool debug = false;       // set to true to show the debug information
    
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_H_
