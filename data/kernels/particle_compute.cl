
#include "a2e_cl_global.h"

__kernel AUTO_VEC_HINT void particle_compute(const float time_passed, const float living_time, const uint particle_count, const float4 gravity,
											 __global float4* pos_time_buffer, __global float4* dir_buffer) {
	int particle_num = get_global_id(0);
	
	float4 pos_time = pos_time_buffer[particle_num];
	float4 dir_vel = dir_buffer[particle_num];
	
	float tpassed = time_passed;
	if(pos_time.w > 0.0f && pos_time.w <= living_time) {
		if(living_time - time_passed < pos_time.w) tpassed = living_time - pos_time.w;
		float time_step = tpassed / 1000.0f;
		float4 acceleration = gravity * time_step;
		dir_vel += acceleration;
		pos_time.xyz += dir_vel.xyz * time_step;
	}
	
	pos_time.w -= time_passed;
	
	pos_time_buffer[particle_num] = pos_time;
	dir_buffer[particle_num] = dir_vel;
}
