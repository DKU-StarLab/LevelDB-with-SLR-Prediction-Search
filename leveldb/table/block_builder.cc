// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// BlockBuilder generates blocks where keys are prefix-compressed:
//
// When we store a key, we drop the prefix shared with the previous
// string.  This helps reduce the space requirement significantly.
// Furthermore, once every K keys, we do not apply the prefix
// compression and store the entire key.  We call this a "restart
// point".  The tail end of the block stores the offsets of all of the
// restart points, and can be used to do a binary search when looking
// for a particular key.  Values are stored as-is (without compression)
// immediately following the corresponding key.
//
// An entry for a particular key-value pair has the form:
//     shared_bytes: varint32
//     unshared_bytes: varint32
//     value_length: varint32
//     key_delta: char[unshared_bytes]
//     value: char[value_length]
// shared_bytes == 0 for restart points.
//
// The trailer of the block has the form:
//     restarts: uint32[num_restarts]
//     num_restarts: uint32
// restarts[i] contains the offset within the block of the ith restart point.

#include "table/block_builder.h"

#include <algorithm>
#include <cassert>

#include "leveldb/comparator.h"
#include "leveldb/options.h"
#include "util/coding.h"

#include<iostream>
using namespace std;

namespace leveldb {

BlockBuilder::BlockBuilder(const Options* options)
    : options_(options), restarts_(), counter_(0), finished_(false) {
  assert(options->block_restart_interval >= 1);
  restarts_.push_back(0);  // First restart point is at offset 0
}

void BlockBuilder::Reset() {
  buffer_.clear();
  restarts_.clear();
  restarts_.push_back(0);  // First restart point is at offset 0
  counter_ = 0;
  finished_ = false;
  last_key_.clear();
}

size_t BlockBuilder::CurrentSizeEstimate() const {
  // return (buffer_.size() +                       // Raw data buffer
          // restarts_.size() * sizeof(uint32_t) +  // Restart array
          // sizeof(uint32_t));                     // Restart array length
		  
	return (buffer_.size() +                       // Raw data buffer
          restarts_.size() * (sizeof(uint32_t) * new_meta_size )+  // Restart array
          sizeof(uint32_t));   
}

static inline const char* DecodeEntry(const char* p, const char* limit,
                                      uint32_t* shared, uint32_t* non_shared,
                                      uint32_t* value_length) {
  if (limit - p < 3) return nullptr;
  *shared = reinterpret_cast<const uint8_t*>(p)[0];
  *non_shared = reinterpret_cast<const uint8_t*>(p)[1];
  *value_length = reinterpret_cast<const uint8_t*>(p)[2];
  if ((*shared | *non_shared | *value_length) < 128) {
    // Fast path: all three values are encoded in one byte each
    p += 3;
  } else {
    if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
  }

  if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
    return nullptr;
  }
  return p;
}

static inline const uint32_t GetKey(const char* data_,uint32_t res, int pos){
	const char *cursor_low = data_ + res + pos * sizeof(uint32_t);  
	uint32_t region_offset_low = leveldb::DecodeFixed32(cursor_low);
	uint32_t shared_low, non_shared_low, value_length_low;
	const char* key_ptr_low = DecodeEntry(data_ + region_offset_low,data_ + res, &shared_low, &non_shared_low, &value_length_low);

	uint32_t skey_i_low;
	if (key_ptr_low == nullptr || (shared_low != 0)) {
	// cout << "key_ptr KOSONG " << endl;
	}else{
		Slice mid_key_low(key_ptr_low, non_shared_low);
		// cout << "key_ptr_low ada " << endl;
		std::string skey_low= mid_key_low.ToString();
		skey_i_low = stoi(skey_low);
	}
	
	return skey_i_low;
}

Slice BlockBuilder::Finish() {
  // Append restart array
  for (size_t i = 0; i < restarts_.size(); i++) {
	PutFixed32(&buffer_, restarts_[i]);
  }
  
  //SLR Segmentation
  int segment_size;
  int low = 0;
  
  int high = restarts_.size() -2 ;
  
  uint32_t skey_i_low,skey_i_high,diff,rest;
  int segment_len = 10;
  
  // if SLR is true
  // and if the keys is quite big
  // save the segments
  if(high > 50 && SLR ){   
	  //this is restart_offset or restarts_ from block.cc
	  uint32_t helper = buffer_.size() + (sizeof(uint32_t) * new_meta_size);
	  uint32_t res = helper - (new_meta_size + restarts_.size()) * sizeof(uint32_t);
	  
	  // buffer_.data() is data_ in block.cc
	  const char* data_ = buffer_.data();
	  
	  
	  //Create Segments
	  segment_size = (high-low)/segment_len;
	  
	  int segment_no=1;
	  int s= low;
	  for(int seg=1; seg <= segment_len; seg ++){
		  
		  int hi= s + segment_size;
		  int first_key= GetKey(data_, res, s);
		  
		  if(hi > high){
			  hi=high;
		  }
		  int last_key= GetKey(data_, res, hi);
		 
		  int dividend=last_key - first_key;
		  int divisor = hi-s;
		  
		  // float coefs = (float)(last_key - first_key)/(divisor);
		  
		  //save into tails encoded
			PutFixed32(&buffer_, dividend);
			PutFixed32(&buffer_, divisor);
			PutFixed32(&buffer_, first_key);
		
			// if(debug){
			  // printf("segment %d low %d\n",seg,s);
			  // printf("segment %d hi %d\n",seg,hi);
			  // printf("segment %d high %d\n",seg,high);
			  // printf("segment %d first_key %d\n",seg,first_key);
			  // printf("segment %d last_key %d\n",seg,last_key);
			  // printf("segment %d coef %.5f\n",seg,coefs);
			  // printf("segment %d size %d\n",seg,divisor + 1);
			  // printf("segment %d dividend %d\n",seg,dividend);
			  // printf("segment %d divisor %d\n\n",seg,divisor);
			  // printf("\n");
			  // printf("\n");
			// }
		  
		  s = hi+1;
	  }
		  
  }else{
	  //set it as zero if SLR isn't active
	  for(int seg=1; seg <= segment_len; seg ++){
		  PutFixed32(&buffer_, 0);
		  PutFixed32(&buffer_, 0);
		  PutFixed32(&buffer_, 0);
	  }
  } 
  
  
  //End Segmentation
  
  //save segment_size as meta
  PutFixed32(&buffer_, segment_size+1);
  PutFixed32(&buffer_, restarts_.size());
 
  finished_ = true;
  return Slice(buffer_);
}

void BlockBuilder::Add(const Slice& key, const Slice& value) {
  Slice last_key_piece(last_key_);
  assert(!finished_);
  assert(counter_ <= options_->block_restart_interval);
  assert(buffer_.empty()  // No values yet?
         || options_->comparator->Compare(key, last_key_piece) > 0);
  size_t shared = 0;
  if (counter_ < options_->block_restart_interval) {
    // See how much sharing to do with previous string
    const size_t min_length = std::min(last_key_piece.size(), key.size());
    while ((shared < min_length) && (last_key_piece[shared] == key[shared])) {
      shared++;
    }
  } else {
    // Restart compression
    restarts_.push_back(buffer_.size());
    counter_ = 0;
  }
  const size_t non_shared = key.size() - shared;

  // Add "<shared><non_shared><value_size>" to buffer_
  PutVarint32(&buffer_, shared);
  PutVarint32(&buffer_, non_shared);
  PutVarint32(&buffer_, value.size());

  // Add string delta to buffer_ followed by value
  buffer_.append(key.data() + shared, non_shared);
  buffer_.append(value.data(), value.size());

  // Update state
  last_key_.resize(shared);
  last_key_.append(key.data() + shared, non_shared);
  assert(Slice(last_key_) == key);
  counter_++;
}

}  // namespace leveldb
