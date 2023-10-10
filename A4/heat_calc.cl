__kernel
void
heat_diff(
	__global const float *matrix_prev,
	const float diff_const,
	__global float *matrix_next,
	int width
	)
{
	int ix = get_global_id(0);
	int iy = get_global_id(1);
	
	int idx = ix * width + iy;

	float value = 0;
	matrix_next[idx] = matrix_prev[idx] + diff_const * ((matrix_prev[(ix - 1) * width + iy] + matrix_prev[(ix + 1) * width + iy] + matrix_prev[ix * width + iy - 1] + matrix_prev[ix * width + iy + 1])/4 - matrix_prev[idx]);
}
