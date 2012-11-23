
#ifndef __PARTICLE_INIT_H__
#define __PARTICLE_INIT_H__

#include "a2e_cl_global.h"

float2 sincos_f2(const float val);
float2 sincos_f2(const float val) {
	return (float2)(sin(val), cos(val));
}

void init_particles(float8* ret, uint* kernel_seed,
					const uint type, const float spawn_rate_ts, const float living_time, const uint particle_count, const float velocity,
					const float4 angle, const float4 extents, const float4 direction, const float4 position_offset);
void init_particles(float8* ret, uint* kernel_seed,
					const uint type, const float spawn_rate_ts, const float living_time, const uint particle_count, const float velocity,
					const float4 angle, const float4 extents, const float4 direction, const float4 position_offset) {
	int particle_num = get_global_id(0);
	float ltime = spawn_rate_ts;

	float4 position = (float4)0.0f;
	float4 dir;
	// output buffers (position, direction, velocity, living_time)
	switch(type) {
		case 0: { // box emitter
			float4 rand = (float4)(sfrand_m1_1(kernel_seed), sfrand_m1_1(kernel_seed), sfrand_m1_1(kernel_seed), 0.0f);
			position.xyz = 0.5f * rand.xyz * extents.xyz;
		}
		break;
		case 1: { // sphere emitter
			float theta = 2.0f * A2E_PI * sfrand_0_1(kernel_seed); // 0 <= theta <= 2pi
			float u = sfrand_m1_1(kernel_seed);
			//float squ = sqrt(1.0f - u*u);
			position = (float4)(cos(theta) * u, sin(theta) * u, u, 0.0f);
			
			float scale = sfrand_m1_1(kernel_seed); // "z"/scale inside sphere
			position.xyz *= extents.x * scale;
		}
		break;
		case 2: // point emitter
		default:
			position = (float4)(0.0f);
			break;
	}
	
	position += position_offset;
	
	if(spawn_rate_ts != 0.0f) {
		float particle_batch_ts = floor(particle_num / spawn_rate_ts);
		ltime = living_time + particle_batch_ts * 40.0f + ((particle_num / spawn_rate_ts) - particle_batch_ts) * 40.0f; // 1000 ms / 25 iterations = 40 ms
	}
	
	// rotate direction
	float4 rangle = (float4)(sfrand_m1_1(kernel_seed), sfrand_m1_1(kernel_seed), sfrand_m1_1(kernel_seed), 0.0f) * angle;
	float4 sina = sin(rangle);
	float4 cosa = cos(rangle);
	dir.x = dot((float4)(cosa.y * cosa.z + sina.x * sina.y * sina.z, -cosa.x * sina.z, sina.x * cosa.y * sina.z - sina.y * cosa.z, 0.0f), direction);
	dir.y = dot((float4)(cosa.y * sina.z - sina.x * sina.y * cosa.z, cosa.x * cosa.z, -sina.y * sina.z - sina.x * cosa.y * cosa.z, 0.0f), direction);
	dir.z = dot((float4)(cosa.x * sina.y, sina.x, cosa.x * cosa.y, 0.0f), direction);
	dir *= velocity;
	
	*ret = (float8)(position.x, position.y, position.z, ltime, dir.x, dir.y, dir.z, 0.0f);
}

#endif
