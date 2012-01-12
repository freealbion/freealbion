
#include "a2e_cl_global.h"

//
kernel void compute_distances(global const float4* pos_buffer, const float4 camera_pos, global float* dist_buffer) {
	const uint global_id = get_global_id(0);
	const float4 pos = pos_buffer[global_id];
	dist_buffer[global_id] = fast_distance(pos.xyz, camera_pos.xyz);
}

/*
 * Copyright 1993-2010 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

inline void ComparatorPrivate(global const float* dist_buffer,
							  uint* keyA,
							  uint* keyB,
							  uint arrowDir) {
	const float dist_a = dist_buffer[*keyA];
	const float dist_b = dist_buffer[*keyB];
	if((uint)(dist_a < dist_b) == arrowDir) {
		uint t = *keyA; *keyA = *keyB; *keyB = t;
	}
}

inline void ComparatorLocal(global const float* dist_buffer,
							local uint* keyA,
							local uint* keyB,
							uint arrowDir) {
	const float dist_a = dist_buffer[*keyA];
	const float dist_b = dist_buffer[*keyB];
	if((uint)(dist_a < dist_b) == arrowDir) {
		uint t = *keyA; *keyA = *keyB; *keyB = t;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Bitonic sort kernel for large arrays (not fitting into local memory)
////////////////////////////////////////////////////////////////////////////////
//Bottom-level bitonic sort
//Almost the same as bitonicSortLocal with the only exception
//of even / odd subarrays (of LOCAL_SIZE_LIMIT points) being
//sorted in opposite directions
// NOTE: this was bitonicSortLocal1
kernel __attribute__((reqd_work_group_size(LOCAL_SIZE_LIMIT / 2, 1, 1)))
void bitonicSortLocal(global const float* dist_buffer,
					  global uint* d_DstKey,
					  global const uint* d_SrcKey) {
	local uint l_key[LOCAL_SIZE_LIMIT];
	const uint local_id = get_local_id(0);
	const uint group_id = get_group_id(0);
	
	//Offset to the beginning of subarray and load data
	d_SrcKey += group_id * LOCAL_SIZE_LIMIT + local_id;
	d_DstKey += group_id * LOCAL_SIZE_LIMIT + local_id;
	l_key[local_id +					  0] = d_SrcKey[					 0];
	l_key[local_id + (LOCAL_SIZE_LIMIT / 2)] = d_SrcKey[(LOCAL_SIZE_LIMIT / 2)];
	
	uint comparatorI = get_global_id(0) & ((LOCAL_SIZE_LIMIT / 2) - 1);
	
	for(uint size = 2; size < LOCAL_SIZE_LIMIT; size <<= 1){
		//Bitonic merge
		uint dir = (comparatorI & (size / 2)) != 0;
		for(uint stride = size / 2; stride > 0; stride >>= 1){
			barrier(CLK_LOCAL_MEM_FENCE);
			uint pos = 2 * local_id - (local_id & (stride - 1));
			ComparatorLocal(dist_buffer,
							&l_key[pos +	  0],
							&l_key[pos + stride],
							dir);
		}
	}
	
	//Odd / even arrays of LOCAL_SIZE_LIMIT elements
	//sorted in opposite directions
	{
		uint dir = (group_id & 1);
		for(uint stride = LOCAL_SIZE_LIMIT / 2; stride > 0; stride >>= 1){
			barrier(CLK_LOCAL_MEM_FENCE);
			uint pos = 2 * local_id - (local_id & (stride - 1));
			ComparatorLocal(dist_buffer,
							&l_key[pos +	  0],
							&l_key[pos + stride],
							dir);
		}
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);
	d_DstKey[					 0]  = l_key[local_id +                      0];
	d_DstKey[(LOCAL_SIZE_LIMIT / 2)] = l_key[local_id + (LOCAL_SIZE_LIMIT / 2)];
}

//Bitonic merge iteration for 'stride' >= LOCAL_SIZE_LIMIT
kernel void bitonicMergeGlobal(global const float* dist_buffer,
							   global uint* d_DstKey,
							   global uint* d_SrcKey,
							   uint arrayLength,
							   uint size,
							   uint stride) {
	uint global_comparatorI = get_global_id(0);
	uint		comparatorI = global_comparatorI & (arrayLength / 2 - 1);
	
	//Bitonic merge
	uint dir = 1 ^ ( (comparatorI & (size / 2)) != 0 );
	uint pos = 2 * global_comparatorI - (global_comparatorI & (stride - 1));
	
	uint keyA = d_SrcKey[pos +      0];
	uint keyB = d_SrcKey[pos + stride];
	
	ComparatorPrivate(dist_buffer,
					  &keyA,
					  &keyB,
					  dir);
	
	d_DstKey[pos +	  0] = keyA;
	d_DstKey[pos + stride] = keyB;
}

//Combined bitonic merge steps for
//'size' > LOCAL_SIZE_LIMIT and 'stride' = [1 .. LOCAL_SIZE_LIMIT / 2]
kernel __attribute__((reqd_work_group_size(LOCAL_SIZE_LIMIT / 2, 1, 1)))
void bitonicMergeLocal(global const float* dist_buffer,
					   global uint* d_DstKey,
					   global uint* d_SrcKey,
					   uint arrayLength,
					   uint size,
					   uint stride) {
	local uint l_key[LOCAL_SIZE_LIMIT];
	const uint local_id = get_local_id(0);
	const uint group_id = get_group_id(0);
	
	d_SrcKey += group_id*  LOCAL_SIZE_LIMIT + local_id;
	d_DstKey += group_id*  LOCAL_SIZE_LIMIT + local_id;
	l_key[local_id +					  0] = d_SrcKey[					 0];
	l_key[local_id + (LOCAL_SIZE_LIMIT / 2)] = d_SrcKey[(LOCAL_SIZE_LIMIT / 2)];
	
	//Bitonic merge
	uint comparatorI = get_global_id(0) & ((arrayLength / 2) - 1);
	uint		 dir = 1 ^ ( (comparatorI & (size / 2)) != 0 );
	for(; stride > 0; stride >>= 1){
		barrier(CLK_LOCAL_MEM_FENCE);
		uint pos = 2*  local_id - (local_id & (stride - 1));
		ComparatorLocal(dist_buffer,
						&l_key[pos +	  0],
						&l_key[pos + stride],
						dir);
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);
	d_DstKey[					 0]  = l_key[local_id +					  0];
	d_DstKey[(LOCAL_SIZE_LIMIT / 2)] = l_key[local_id + (LOCAL_SIZE_LIMIT / 2)];
}

