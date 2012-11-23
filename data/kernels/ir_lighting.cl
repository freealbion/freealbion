
#include "a2e_cl_global.h"
#include "a2e_cl_matrix.h"

//#define A2E_IR_LIGHT_CULLING 1

#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable

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

bool light_intersects(global const struct ir_light* lights, const float2 minmax_depth, const unsigned int light_index);
bool light_intersects(global const struct ir_light* lights, const float2 minmax_depth, const unsigned int light_index) {
	return true;
}

// defines: A2E_IR_TILE_SIZE_X, A2E_IR_TILE_SIZE_Y
#define A2E_IR_TILE_SIZE (A2E_IR_TILE_SIZE_X*A2E_IR_TILE_SIZE_Y)
kernel void ir_lighting(
						/* in textures: */
						read_only image2d_t normal_nuv_buffer,
						read_only image2d_t depth_buffer,
						
						/* out textures: */
						write_only image2d_t output_buffer_diff,
						write_only image2d_t output_buffer_spec,
						
						/* general parameters: */
						const float3 cam_position, const uint2 screen_size,
						const float2 projection_ab, const matrix4 IMVM,
						const uint2 tiles,
						
						/* lights: */
						constant const struct ir_light* lights,
						const unsigned int light_count,
						
						/* debugging */
						global unsigned int* dbg_buffer
						) {
	//
#if defined(A2E_IR_LIGHT_CULLING)
	volatile local unsigned int visible_light_count = 0;
	volatile local unsigned int min_depth = 0xFFFFFFFF;
	volatile local unsigned int max_depth = 0;
	volatile local unsigned int visible_light_indices[128];
	//write_mem_fence(CLK_LOCAL_MEM_FENCE);
	/*atomic_xchg(&visible_light_count, 0);
	atomic_xchg(&min_depth, 0xFFFFFFFF);
	atomic_xchg(&max_depth, 0);*/
	visible_light_count = 0;
	min_depth = 0xFFFFFFFF;
	max_depth = 0;
	//barrier(CLK_LOCAL_MEM_FENCE);
	write_mem_fence(CLK_LOCAL_MEM_FENCE);
	//mem_fence(CLK_LOCAL_MEM_FENCE);
#endif
	
	const unsigned int local_id = get_local_id(0);
	const unsigned int group_id = get_group_id(0);
	const unsigned int idx = (group_id % tiles.x) * A2E_IR_TILE_SIZE_X + (local_id % A2E_IR_TILE_SIZE_X);
	const unsigned int idy = (group_id / tiles.x) * A2E_IR_TILE_SIZE_Y + (local_id / A2E_IR_TILE_SIZE_X);
	if(idx >= screen_size.x || idy >= screen_size.y) return; // TODO
	
	const int2 tex_coord = (int2)(idx, idy);
	const sampler_t point_sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_FILTER_NEAREST | CLK_ADDRESS_NONE;
	
	////////////////////////////////////////////////////////////////////////////
	//
	const float2 fscreen_size = (float2)(screen_size.x, screen_size.y);
	float2 ftex_coord = (float2)(idx, idy) / fscreen_size;
	
	float depth = read_imagef(depth_buffer, point_sampler, tex_coord).x; // in [0, 1]
	if(depth >= 1.0f) return; // TODO
	
#if defined(A2E_IR_LIGHT_CULLING)
	// compute the min and max depth of this tile
	// for local memory atomics capable devices:
	const unsigned int uint_depth = as_uint(depth);
	atomic_min(&min_depth, uint_depth);
	atomic_max(&max_depth, uint_depth);
	//barrier(CLK_LOCAL_MEM_FENCE);
	//mem_fence(CLK_LOCAL_MEM_FENCE);
#endif
	
	float4 norm_nuv_read = read_imagef(normal_nuv_buffer, point_sampler, tex_coord);
	float2 normal_read = norm_nuv_read.xy;
	float2 nuv = norm_nuv_read.zw;
	
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
	
	// this is used by surfaces that should always be lit when there is a light nearby
	if(nuv.x == 0.0f && nuv.y == 0.0f) {
		//light_dir = view_dir; // TODO: -> !!!
		// bend the normal by a small amount to prevent singularities in the light computation
		normal = normalize(view_dir + (float3)(0.01f, -0.01f, 0.01f));
		nuv = (float2)(1.0f, 1.0f);
	}
	
	////////////////////////////////////////////////////////////////////////////
	// lighting
	float4 diff_color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec_color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
	
	// light culling
#if defined(A2E_IR_LIGHT_CULLING)
	barrier(CLK_LOCAL_MEM_FENCE); // from earlier atomic min/max
	float2 minmax_depth = (float2)(as_float(min_depth), as_float(max_depth));
	
	/*barrier(CLK_LOCAL_MEM_FENCE);
	atomic_xchg(&visible_light_count, 0);
	barrier(CLK_LOCAL_MEM_FENCE);*/
	
	// light_index == local_id
	//unsigned int max_offset = 0;
	if(local_id < light_count) {
		if(light_intersects(lights, minmax_depth, local_id)) {
			//const unsigned int offset = atomic_inc(&visible_light_count);
			//visible_light_indices[min(offset, 127u)] = local_id;
			visible_light_indices[atomic_inc(&visible_light_count)] = local_id;
			//if(offset > max_offset) max_offset = offset;
			//atomic_xchg(&visible_light_indices[offset], local_id);
		}
	}
	//atomic_min(&visible_light_count, light_count);
	barrier(CLK_LOCAL_MEM_FENCE);
	//write_mem_fence(CLK_LOCAL_MEM_FENCE);
	//mem_fence(CLK_LOCAL_MEM_FENCE);
#endif
	
	/*for(unsigned int i = 0; i < visible_light_count; i++) {
		const unsigned int light_index = visible_light_indices[i];
		const float4 lpos = lights[light_index].position;
		const float4 lcolor = lights[light_index].color;
		
		float3 light_dir = (lpos.xyz - rep_pos) * lcolor.w;
		float attenuation = 1.0f - dot(light_dir, light_dir) * (lpos.w * lpos.w);
		if(attenuation <= 0.0f) continue;
		
		light_dir = normalize(light_dir);
		
		// phong lighting
		float lambert_term = dot(normal, light_dir);
		if(lambert_term > 0.001f) {
			color.xyz += lcolor.xyz * lambert_term * attenuation;
			// TODO: specular
		}
	}*/
	
	
	for(unsigned int i = 0; i < light_count; i++) {
	//dbg_buffer[local_id] = visible_light_indices[min(loop_light_count, local_id)];
	//dbg_buffer[local_id] = (local_id < light_count ? atomic_inc(&visible_light_count) : 0);
	//dbg_buffer[local_id] = max_offset == 0 ? visible_light_count : max_offset;
	/*for(unsigned int i = 0; i < visible_light_count; i++) {
		const unsigned int light_index = visible_light_indices[i];
		const float4 light_position = lights[light_index].position;
		const float4 light_color = lights[light_index].color;*/
		
		const float4 light_position = lights[i].position;
		const float4 light_color = lights[i].color;
		
		float3 light_dir = (light_position.xyz - rep_pos);
		light_dir *= light_color.w;
		float attenuation = 1.0f - dot(light_dir, light_dir) * (light_position.w * light_position.w);
		if(attenuation <= 0.01f) continue;
		light_dir = normalize(light_dir);
		
		// ashikhmin-shirley
		float3 half_vec = normalize(light_dir + view_dir);
		
		//
		float3 epsilon = (float3)(1.0f, 0.0f, 0.0f);
		float3 tangent = normalize(cross(normal, epsilon));
		float3 bitangent = normalize(cross(normal, tangent));
		
		//
		float LdotN = dot(light_dir, normal);
		if(LdotN <= 0.001f) continue;
		
		float VdotN = dot(view_dir, normal);
		float HdotN = dot(half_vec, normal);
		float HdotL = dot(half_vec, light_dir);
		float HdotT = dot(half_vec, tangent);
		float HdotB = dot(half_vec, bitangent);
		
		float3 light_rgb = light_color.xyz * attenuation;
		
		// compute diffuse
		const float pd_const = 28.0f / (23.0f * 3.14159f);
		float pd_0 = 1.0f - pow(1.0f - (LdotN / 2.0f), 5.0f);
		float pd_1 = 1.0f - pow(1.0f - (VdotN / 2.0f), 5.0f);
		diff_color.xyz += pd_const * pd_0 * pd_1 * light_rgb;
		
		// compute specular
		float ps_num_exp = nuv.x * HdotT * HdotT + nuv.y * HdotB * HdotB;
		ps_num_exp /= (1.0f - HdotN * HdotN);
		float ps_num = sqrt((nuv.x + 1.0f) * (nuv.y + 1.0f));
		ps_num *= pow(HdotN, ps_num_exp);
		
		float ps_den = (8.0f * 3.14159f) * HdotL * max(LdotN, VdotN);
		
		// store main rgb component in spec rgb and partial schlick term in alpha
		spec_color.xyz += light_rgb * (ps_num / ps_den);
		spec_color.w += pow(1.0f - HdotL, 5.0f);
	}
	
	write_imagef(output_buffer_diff, tex_coord, diff_color);
	write_imagef(output_buffer_spec, tex_coord, spec_color);
}
