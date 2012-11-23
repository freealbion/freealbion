
#ifndef __A2E_GLOBAL_H__
#define __A2E_GLOBAL_H__

// workaround for the nvidia compiler ...
#if defined(UNDEF__APPLE__)
#undef __APPLE__
#endif

#if !defined(CPU) && !defined(OCLRASTER_CUDA_CL)
#undef printf
#define printf(x, ...)
#endif

#if !defined(OCLRASTER_CUDA_CL)
#define OCLRASTER_FUNC inline
#endif

#if defined(PLATFORM_NVIDIA)
#pragma OPENCL EXTENSION cl_nv_compiler_options : enable
#endif

#if defined(PLATFORM_APPLE)
// TODO: in recent os x version this has be changed to cl_khr_gl_sharing, even though it's not
// explicitly supported/exposed by the driver/runtime
//#pragma OPENCL EXTENSION cl_APPLE_gl_sharing : enable
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#else
// note: amd devices support this, but don't expose the extension and won't compile if this is enabled
#if !defined(PLATFORM_AMD)
#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
#endif
#endif

#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

#define A2E_PI 3.1415926535897932384626433832795f

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
