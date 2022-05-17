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
	  // if(restarts_.size() > 50){
		  // cout << "pkkkk " << i << " => " << restarts_[i] << endl;
	  // }
    PutFixed32(&buffer_, restarts_[i]);
  }
  
  // PutFixed32(&buffer_, 55);
  // cout << "restarts_.size() " << restarts_.size() << endl;
  // if(restarts_.size() > 50){
	  // PutFixed32(&buffer_, 44);
  // }
  
  //uint32_t mid = (left + right + 1) / 2;
  // DecodeFixed32(data_ + restarts_ + restarts_.size() * sizeof(uint32_t));
  //uint32_t region_offset = DecodeFixed32(  restarts_.size() * sizeof(uint32_t));
  // const char* aku = "\x40";
  // uint32_t pertama = 1635;
  // uint32_t helper = buffer_.size() + sizeof(uint32_t) + sizeof(uint32_t);
  // // uint32_t pertama2 = buffer_.size() - (2 + restarts_.size()) * sizeof(uint32_t);
  // uint32_t pertama = helper - (2 + restarts_.size()) * sizeof(uint32_t);
  // //this is restart_offset or restarts_ from block.cc
  
  // // cout << "pertama " << pertama << endl;
  
  // uint32_t kedua = 33;
  // const char* aku = "\x34\03";
  // // const char* const data_ = pertama + kedua * sizeof(uint32_t) ; 
  // // const char *cursor = restarts_.data() + pertama + kedua * sizeof(uint32_t) ; 
  // // const char *cursor = aku + pertama + kedua * sizeof(uint32_t); 
  
  
  // // buffer_.data() is data_ in block.cc
  // const char *cursor = buffer_.data() + pertama + kedua * sizeof(uint32_t);  
	// uint32_t region_offset = leveldb::DecodeFixed32(cursor);
  // uint32_t shared, non_shared, value_length;
  
  // const char* key_ptr = DecodeEntry(buffer_.data() + region_offset, buffer_.data() + pertama, &shared, &non_shared, &value_length);
  // // const char* key_ptr = DecodeEntry(data_ + region_offset, data_ + restarts_, &shared, &non_shared, &value_length);
  // // uint32_t restart_offset_ = size_ - (2 + restarts_.size()) * sizeof(uint32_t);
  // // uint32_t restart_offset_ = (2 + restarts_.size()) * sizeof(uint32_t);
  // uint32_t skey_i;
  // if (key_ptr == nullptr || (shared != 0)) {
        // cout << "key_ptr KOSONG " << endl;
  // }else{
	  // Slice mid_key(key_ptr, non_shared);
	  // cout << "key_ptr ada " << endl;
	  // std::string skey= mid_key.ToString();
	  // skey_i = stoi(skey);
		// cout << "mid_key " << skey << endl;
		// cout << "key_ptr " << key_ptr << endl;
		// cout << "skey_i " << skey_i << endl;
  // }
  
  
  //LR Prediction Preparation
  
  
 
  
  // uint32_t low = 33;
  int segment_size;
  int low = 0;
  // uint32_t high = restarts_.size();
  // uint32_t high = 64;
  
  int high = restarts_.size() -2 ;
  // cout << "high2 " << high << endl;    
  // cout << "SLR Active Build: " << SLR << endl;    
  
  uint32_t skey_i_low,skey_i_high,diff,rest;
  int segment_len = 10;
  if(high > 50 && SLR ){   
		
	  // uint32_t helper = buffer_.size() + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
	  // uint32_t res = helper - (3 + restarts_.size()) * sizeof(uint32_t);
	  
	  uint32_t helper = buffer_.size() + (sizeof(uint32_t) * new_meta_size);
	  uint32_t res = helper - (new_meta_size + restarts_.size()) * sizeof(uint32_t);
	  
	  //this is restart_offset or restarts_ from block.cc
	  // cout << "restarts_.size() " << restarts_.size() << endl;
	  // buffer_.data() is data_ in block.cc
	  const char* data_ = buffer_.data();
	  
	  
	  //Create Segments
		// int total_segment=6;
		  
		  segment_size = (high-low)/segment_len;
		  // printf("segment_size %d\n",segment_size);
		  
		  int segment_no=1;
		  int s= low;
		  for(int seg=1; seg <= segment_len; seg ++){
			  
			  int hi= s + segment_size;
			  int first_key= GetKey(data_, res, s);
			  
			  
			  if(hi > high){
				  // printf("upgraded %d hi %d\n",seg,hi);
				  hi=high;
				  
			  }
			  int last_key= GetKey(data_, res, hi);
			  // int diff = hi-s + 1;
			  
			  int dividend=last_key - first_key;
			  int divisor = hi-s;
			  
			  // float coefs = (float)(last_key - first_key)/(diff - 1);
			  float coefs = (float)(last_key - first_key)/(divisor);
			  
			
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
		  // for(int s=low;s < high; s+=segment_size){
			  // int hi= s + segment_size;
			  // // int first_key= myarray[s];
			  // int first_key= GetKey(data_, res, s);
			  // if(hi > high){
				  // hi=high;
				  // segment_size=hi-s;
			  // }
			  // // int last_key= myarray[hi];
			  // int last_key= GetKey(data_, res, hi);
			  
			  
			  
			  // float coefs = (float)(last_key - first_key)/(segment_size - 1);
				  
			  // printf("segment %d low %d\n",segment_no,s);
			  // printf("segment %d high %d\n",segment_no,high);
			  // printf("segment %d first_key %d\n",segment_no,first_key);
			  // printf("segment %d last_key %d\n",segment_no,last_key);
			  // printf("segment %d coef %.5f\n",segment_no,coefs);
			  // printf("segment %d size %d\n\n",segment_no,segment_size);
			  
			  // // segment[segment_no]=first_key;
			  // // segment_coef[segment_no]=coefs;
			  // // segment_add[segment_no]=s;
			  
				// segment_no++;
			  
		  // }
		  
		  //Create Segments
	  
	  // //percobaan
	  // for(int k=0; k< high -1 ; k++){
		  // const char *cursor_low = data_ + res + k * sizeof(uint32_t);  
		  // uint32_t region_offset_low = leveldb::DecodeFixed32(cursor_low);
		  // uint32_t shared_low, non_shared_low, value_length_low;
		  // const char* key_ptr_low = DecodeEntry(data_ + region_offset_low,data_ + res, &shared_low, &non_shared_low, &value_length_low);
		  
		  
		  // if (key_ptr_low == nullptr || (shared_low != 0)) {
			// // cout << "key_ptr KOSONG " << endl;
		  // }else{
			// Slice mid_key_low(key_ptr_low, non_shared_low);
			// // cout << "key_ptr_low ada " << endl;
			// std::string skey_low= mid_key_low.ToString();
			// cout << "badts " << k << " => " << skey_low << endl;
			
		  // }
	  // }
	  // //percobaan
	  
	  /*
	  // get data for lowest subset
	  const char *cursor_low = data_ + res + low * sizeof(uint32_t);  
	  uint32_t region_offset_low = leveldb::DecodeFixed32(cursor_low);
	  uint32_t shared_low, non_shared_low, value_length_low;
	  const char* key_ptr_low = DecodeEntry(data_ + region_offset_low,data_ + res, &shared_low, &non_shared_low, &value_length_low);
	  
	  
	  if (key_ptr_low == nullptr || (shared_low != 0)) {
		// cout << "key_ptr KOSONG " << endl;
	  }else{
		Slice mid_key_low(key_ptr_low, non_shared_low);
		// cout << "key_ptr_low ada " << endl;
		std::string skey_low= mid_key_low.ToString();
		skey_i_low = stoi(skey_low);
		
	  }
	  // get data for lowest subset
	  
	  // get data for highest subset
	  const char *cursor_high = data_ + res + high * sizeof(uint32_t);  
	  uint32_t region_offset_high = leveldb::DecodeFixed32(cursor_high);
	  uint32_t shared_high, non_shared_high, value_length_high;
	  const char* key_ptr_high = DecodeEntry(data_ + region_offset_high,data_ + res, &shared_high, &non_shared_high, &value_length_high);
	  
	  // uint32_t skey_i_high;
	  if (key_ptr_high == nullptr || (shared_high != 0)) {
		
	  }else{
		Slice mid_key_high(key_ptr_high, non_shared_high);
		
		std::string skey_high= mid_key_high.ToString();
		
		cout << "skey_highxxx " << skey_high << endl;
		
		skey_i_high = stoi(skey_high);
		
	  }
	  // get data for highest subset
	  
	  if(skey_i_low && skey_i_high){
			
			int low_length =  std::to_string(skey_i_low).length();
			int high_length =  std::to_string(skey_i_high).length();
			
			cout << "#!-----------" << endl;
			cout << "region_offset_high " << region_offset_high << endl;
			cout << "low_length " << low_length << endl;
			cout << "high_length " << high_length << endl;
			cout << "skey_i_high " << skey_i_high << endl;
			cout << "skey_i_low " << skey_i_low << endl;
			
			
			if(low_length == high_length){
				
			}else{
				if(high_length > low_length){
					rest = high_length - low_length;
					skey_i_low = skey_i_low * (rest * 10);
					cout << "new skey_i_low " << skey_i_low << endl;
				}else{
					rest = low_length - high_length;
					skey_i_high = skey_i_high * (rest * 10);
					cout << "new skey_i_high " << skey_i_high << endl;
				}
			}
			
			diff =  skey_i_high - skey_i_low;
			
			
			cout << "diff " << diff << endl;
			cout << "-----------!#" << endl;
			
	  }
	  
	  */
  }else{
	  for(int seg=1; seg <= segment_len; seg ++){
		  PutFixed32(&buffer_, 0);
		  PutFixed32(&buffer_, 0);
		  PutFixed32(&buffer_, 0);
	  }
  } 
  
  
  //LR Prediction Preparation
  
  
  
  
  // std::string hasil = mid_key.ToString();
  // uint32_t hasil_int = (int)hasil;
  // cout << "mid_key key_ptr " << key_ptr << endl; 
  // printf("kye_pttttr %c \n", key_ptr);
  // std::string skey= mid_key.ToString();
	// cout << "mid_key " << skey << endl;
	// cout << "region_offset " << region_offset << endl;
	// cout << "pertama " << pertama << endl;
	// cout << "key_ptr " << key_ptr << endl;
   // float myFloatNum = 5.99f;   
  
  //fill segment size
  // PutFixed32(&buffer_, 10); // div 1
  // PutFixed32(&buffer_, 10); // div 2
  // PutFixed32(&buffer_, 10); // div 3
  // PutFixed32(&buffer_, 10); // div 4
  // PutFixed32(&buffer_, 10); // div 5
  // //fill segment size  
  
  // //fill dividends
  // PutFixed32(&buffer_, skey_i_low); // div 1
  // PutFixed32(&buffer_, skey_i_low); // div 2
  // PutFixed32(&buffer_, skey_i_low); // div 3
  // PutFixed32(&buffer_, skey_i_low); // div 4
  // PutFixed32(&buffer_, skey_i_low); // div 5
  // //fill dividends
  
  // //fill lowest keys
  // // PutFixed32(&buffer_, diff); // low 1
  // PutFixed32(&buffer_, 1); // low 1
  // PutFixed32(&buffer_, 2); // low 2
  // PutFixed32(&buffer_, 3); // low 3
  // PutFixed32(&buffer_, 4); // low 4
  // PutFixed32(&buffer_, 5); // low 5 
  // //fill lowest keys
  
  // PutLengthPrefixedSlice(&buffer_, Slice("5.99"));
  // PutFixed32(&buffer_, myFloatNum);
 
  // PutFloat32(&buffer_, myFloatNum);
  
  // PutFixed32(&buffer_, 0x40b33333);
  PutFixed32(&buffer_, segment_size+1);
  PutFixed32(&buffer_, restarts_.size());
 
  // if(restarts_.size() > 50){
	  // // PutFixed32(&buffer_, 44);
  // }
  
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
