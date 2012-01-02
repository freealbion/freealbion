
#ifndef __A2E_GLOBAL_CLH__
#define __A2E_GLOBAL_CLH__

// workaround for the nvidia compiler ...
#ifdef UNDEF__APPLE__
#undef __APPLE__
#endif

#ifndef CPU // gpu mode
#define printf(x, ...)
#else // cpu mode
//
#endif

#ifdef NVIDIA
#pragma OPENCL EXTENSION cl_nv_compiler_options : enable
#endif

#ifdef __APPLE__
#pragma OPENCL EXTENSION cl_APPLE_gl_sharing : enable
#else

// note: amd devices support this, but don't expose the extension and won't compile if this is enabled
#if !defined(AMD)
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#endif

#endif

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

#define A2E_PI 3.1415926535897932384626433832795f
#define AUTO_VEC_HINT __attribute__((vec_type_hint(float4)))

// all props go to iñigo quilez: http://iquilezles.org/www/articles/sfrand/sfrand.htm
float sfrand_0_1(uint* seed);
float sfrand_m1_1(uint* seed);

// this results in a random number in [0, 1]
float sfrand_0_1(uint* seed) {
	float res;
	*seed = mul24(*seed, 16807u);
	*((uint*)&res) = ((*seed) >> 9) | 0x3F800000;
	return (res - 1.0f);
}
// this results in a random number in [-1, 1]
float sfrand_m1_1(uint* seed) {
	float res;
	*seed = mul24(*seed, 16807u);
	*((uint*)&res) = ((*seed) >> 9) | 0x40000000;
	return (res - 3.0f);
}

#endif
