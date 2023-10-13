__kernel
void
heat_diff(
	__global const float *matrix_prev,
	const float diff_const,
	__global float *matrix_next,
	int width
	)
{
	// we add one to accunt for our zero border
	int ix = get_global_id(0) + 1;
	int iy = get_global_id(1) + 1;

	int idx = iy*width+ix;
	float value = matrix_prev[(iy -1)*width + ix] + matrix_prev[(iy+1)*width + ix] + matrix_prev[iy*width + ix-1] + matrix_prev[iy*width+ix+1];
    	matrix_next[idx] = matrix_prev[idx] + diff_const * (value/4 - matrix_prev[idx]);
}

__kernel
void 
heat_abs_diff(
	__global float *temps,
	const float average_temp,
	int width 
) {

	// we add one to accunt for our zero border
	int ix = get_global_id(0) + 1;
	int iy = get_global_id(1) + 1;

	// convert to linear index
	int idx = iy * width + ix;

	// overwrite temps with the absolute difference to the average temp
	temps[idx] = fabs(temps[idx] - average_temp);

}

__kernel
void
compute_reduction(
	__global const float *matrix,
	__local float *scratch,
	__const int sz,
	__global float *result
	)
{
	int gsz = get_global_size(0);
	int gix = get_global_id(0);
	int lsz = get_local_size(0);
	int lix = get_local_id(0);

	float acc = 0;
	for ( int idx = get_global_id(0); idx < sz; idx += gsz )
		acc += matrix[idx];

	scratch[lix] = acc;
	barrier(CLK_LOCAL_MEM_FENCE);

	for(int offset = lsz/2; offset > 0; offset /= 2) {
		if ( lix < offset )
			scratch[lix] += scratch[lix+offset];
		barrier(CLK_LOCAL_MEM_FENCE);
		}
	if ( lix == 0 )
		result[get_group_id(0)] = scratch[0];
}
