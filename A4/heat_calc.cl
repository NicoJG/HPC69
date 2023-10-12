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

	//This is to check the boundaries. I don't know if it's affects performance or not. Otherwise we could work with a bigger matrix where the edges are set to zero perhaps. /R

	if (ix > 0) value += matrix_prev[(ix-1)*width + iy];
    	if (ix < width-1) value += matrix_prev[(ix+1)*width + iy];
    	if (iy > 0) value += matrix_prev[ix*width + iy-1];
    	if (iy < width-1) value += matrix_prev[ix*width + iy+1];

    	matrix_next[idx] = matrix_prev[idx] + diff_const * (value/4 - matrix_prev[idx]);
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
