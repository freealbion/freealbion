
#include "a2e_cl_global.h"
#include "a2e_cl_matrix.h"

//#define A2E_IR_LIGHT_CULLING 1

#if defined(A2E_IR_LIGHT_CULLING)
#if defined(A2E_LOCAL_ATOMICS)
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#endif
#endif

#if !defined(A2E_IR_TILE_SIZE_X)
#define A2E_IR_TILE_SIZE_X 8
#endif
#if !defined(A2E_IR_TILE_SIZE_Y)
#define A2E_IR_TILE_SIZE_Y 8
#endif

struct __attribute__((aligned(32), packed)) ir_light {
	float4 position;
	float4 color;
};

float3 reflect(const float3 N, const float3 I);
float3 reflect(const float3 N, const float3 I) {
	return I - 2.0f * dot(N, I) * N;
}

bool light_intersects(global const struct ir_light* light, const float2 minmax_depth, const unsigned int li);
bool light_intersects(global const struct ir_light* light, const float2 minmax_depth, const unsigned int li) {
	return true;
}

// defines: A2E_IR_TILE_SIZE_X, A2E_IR_TILE_SIZE_Y
#define A2E_IR_TILE_SIZE (A2E_IR_TILE_SIZE_X*A2E_IR_TILE_SIZE_Y)
kernel void ir_lighting(
						/* in textures: */
						read_only image2d_t normal_nuv_buffer,
						read_only image2d_t depth_buffer,
						
						/* out textures: */
						write_only image2d_t output_buffer,
						
						/* general parameters: */
						const float3 cam_position, const int2 screen_size,
						const float2 projection_ab, const matrix4 IMVM,
						const int2 tiles,
						
						/* lights: */
						global const struct ir_light* lights,
						const unsigned int light_count
						) {
	//
#if defined(A2E_IR_LIGHT_CULLING)
#if defined(A2E_LOCAL_ATOMICS)
	local unsigned int visible_light_count = 0;
	local unsigned int min_depth = 0xFFFFFFFF;
	local unsigned int max_depth = 0;
	local unsigned int visible_light_indices[64];
#else
	//unsigned int visible_light_count = 0;
	local int visible_light_indices[64];
#endif
	write_mem_fence(CLK_LOCAL_MEM_FENCE);
#endif
	
#if 1
	const size_t local_id = get_local_id(0);
	const size_t group_id = get_group_id(0);
	const size_t idx = (group_id % tiles.x) * A2E_IR_TILE_SIZE_X + (local_id % A2E_IR_TILE_SIZE_X);
	const size_t idy = (group_id / tiles.x) * A2E_IR_TILE_SIZE_Y + (local_id / A2E_IR_TILE_SIZE_X);
	if(idx >= screen_size.x || idy >= screen_size.y) return;
	
	const int2 tex_coord = (int2)(idx, idy);
	const sampler_t point_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
	
	////////////////////////////////////////////////////////////////////////////
	//
	const float2 fscreen_size = (float2)(screen_size.x, screen_size.y);
	float2 ftex_coord = (float2)(idx, idy) / fscreen_size;
	
	float depth = read_imagef(depth_buffer, point_sampler, tex_coord).x; // in [0, 1]
	if(depth >= 1.0f) return;
	
#if defined(A2E_IR_LIGHT_CULLING)
	// compute the min and max depth of this tile
#if defined(A2E_LOCAL_ATOMICS)
	// for local memory atomics capable devices:
	const unsigned int uint_depth = as_uint(depth);
	atom_min(&min_depth, uint_depth);
	atom_max(&max_depth, uint_depth);
	//barrier(CLK_LOCAL_MEM_FENCE);
#else
	// for all others:
	local float tile_depths[A2E_IR_TILE_SIZE];
	
	// write depth of each work-item
	tile_depths[local_id] = depth;
	//write_mem_fence(CLK_LOCAL_MEM_FENCE);
	barrier(CLK_LOCAL_MEM_FENCE);
	
	// compute min/max
	float2 minmax_depth = (float2)(1.0f, 0.0f);
	for(int i = 0; i < A2E_IR_TILE_SIZE; i++) {
		minmax_depth.x = fmin(minmax_depth.x, tile_depths[i]);
		minmax_depth.y = fmax(minmax_depth.y, tile_depths[i]);
	}
#endif
#endif
	
	float4 norm_nuv_read = read_imagef(normal_nuv_buffer, point_sampler, tex_coord);
	float2 normal_read = norm_nuv_read.xy;
	//float2 nuv_read = norm_nuv_read.zw;
	
	////////////////////////////////////////////////////////////////////////////
	// reconstruct world space position
	const float linear_depth = projection_ab.y / (depth - projection_ab.x); // in [near plane, far plane]
	const float up_vec = 0.72654252800536066024136247722454f;
	const float right_vec = up_vec * (fscreen_size.x / fscreen_size.y);
	
	float3 rep_pos;
	rep_pos.z = -linear_depth;
	rep_pos.xy = (ftex_coord * 2.0f - 1.0f) * (float2)(right_vec, up_vec) * linear_depth;
	rep_pos = matrix4_mul_float4(IMVM, (float4)(rep_pos, 1.0f)).xyz;
	
	////////////////////////////////////////////////////////////////////////////
	// decode normal
	float3 normal;
	normal.z = dot(normal_read.xy, normal_read.xy) * 2.0f - 1.0f;
	// and again: abs inner sqrt result, b/c of fp inaccuracies
	normal.xy = normalize(normal_read.xy) * sqrt(fabs(1.0f - normal.z * normal.z));
	
	////////////////////////////////////////////////////////////////////////////
	// light independent
	float3 view_dir = cam_position - rep_pos;
	view_dir = normalize(view_dir);
	
	////////////////////////////////////////////////////////////////////////////
	// lighting
	float4 color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	
#if defined(A2E_IR_LIGHT_CULLING)
	// light culling
#if defined(A2E_LOCAL_ATOMICS)
	barrier(CLK_LOCAL_MEM_FENCE); // from earlier atomic min/max
	float2 minmax_depth = (float2)(as_float(min_depth), as_float(max_depth));
	
	// light_index == local_id
	if(local_id < light_count &&
	   light_intersects(&lights[local_id], minmax_depth, local_id)) {
		unsigned int offset = atom_inc(&visible_light_count);
		visible_light_indices[offset] = local_id;
	}
	barrier(CLK_LOCAL_MEM_FENCE);
	//write_mem_fence(CLK_LOCAL_MEM_FENCE);
#else	
	const unsigned int light_iterations = light_count / A2E_IR_TILE_SIZE + (light_count % A2E_IR_TILE_SIZE > 0 ? 1 : 0);
	for(int iter = 0; iter < light_iterations; iter++) {
		int light_index = iter*A2E_IR_TILE_SIZE + local_id;
		if(light_index < light_count &&
		   light_intersects(&lights[light_index], minmax_depth, light_index)) {
			visible_light_indices[light_index] = light_index;
		}
		else visible_light_indices[light_index] = -1;
	}
	barrier(CLK_LOCAL_MEM_FENCE); // TODO: check if global fence is necessary
	//visible_light_count = wg_visible_light_count[group_id];
	//visible_light_count = light_count;
#endif
#endif
	
#if defined(A2E_IR_LIGHT_CULLING)
#if defined(A2E_LOCAL_ATOMICS)
	for(unsigned int i = 0; i < visible_light_count; i++) {
		const unsigned int light_index = visible_light_indices[i];
#else
	for(unsigned int i = 0; i < light_count; i++) {
		const int light_index = visible_light_indices[i];
		if(light_index < 0) continue;
#endif
#else
	for(unsigned int light_index = 0; light_index < light_count; light_index++) {
#endif
		const float4 lpos = lights[light_index].position;
		const float4 lcolor = lights[light_index].color;
		
		float3 light_dir = (lpos.xyz - rep_pos) * lcolor.w;
		float attenuation = 1.0f - dot(light_dir, light_dir) * lpos.w;
		if(attenuation <= 0.0f) continue;
		
		light_dir = normalize(light_dir);
		
		// phong lighting
		float lambert_term = dot(normal, light_dir);
		if(lambert_term > 0.001f) {
			color.xyz += lcolor.xyz * lambert_term * attenuation;
			// TODO: specular
		}
	}
	
	write_imagef(output_buffer, tex_coord, color);
#endif
}
