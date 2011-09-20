
#ifndef __A2E_MATRIX_CLH__
#define __A2E_MATRIX_CLH__

typedef float16 matrix4;

float4 matrix4_mul_float4(const matrix4 mat, const float4 vec);
matrix4 matrix4_mul_matrix4(const matrix4 mat_1, const matrix4 mat_2);

matrix4 matrix4_mul_matrix4(const matrix4 mat_1, const matrix4 mat_2) {
	matrix4 ret_mat;
	for(size_t i = 0; i < 4; i++) { // column
		for(size_t j = 0; j < 4; j++) { // row
			((float*)&ret_mat)[i*4 + j] = 0.0f;
			for(size_t k = 0; k < 4; k++) { // mul iteration
				((float*)&ret_mat)[i*4 + j] +=
					((const float*)&mat_1)[i*4 + k] * ((const float*)&mat_2)[k*4 + j];
			}
		}
	}
	return ret_mat;
}

float4 matrix4_mul_float4(const matrix4 mat, const float4 vec) {
	float4 ret_vec;
	ret_vec = vec.x * (mat).lo.lo;
	ret_vec += vec.y * (mat).lo.hi;
	ret_vec += vec.z * (mat).hi.lo;
	ret_vec += vec.w * (mat).hi.hi;
	return ret_vec;
}

#endif
