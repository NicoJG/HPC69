__kernel
void
heat_diff(
	__global const float *a,
	const double c,
	__global float *n,
	int width_a
	)
{
	int ix = get_global_id(0);
	int iy = get_global_id(1);
	float value = 0;
	n[ix*width_a+iy] = a[ix*width_a+iy] + c * ((a[(ix-1)*width_a+iy]+a[(ix+1)*width_a+iy]+a[ix*width_a+iy-1]+a[ix*width_a+iy+1])/4-a[ix*width_a+iy]);
}
