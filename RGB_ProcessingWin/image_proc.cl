
//
// Negative image kernels
//


// Input and output images stored as generic memory buffer objects
kernel void negativeImage(global const float *input, global float *output, const int w)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	global const float *srcPixel = input + (y * w) + x;
	global float *dstPixel = output + (y * w) + x;

	*dstPixel = 1.0f - *srcPixel;
}


// Input and output images stored as an OpenCL image object
kernel void negativeImageSampler(read_only image2d_t input, write_only image2d_t output, int w, int h)
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
	
	int x = get_global_id(0);
	int y = get_global_id(1);

	float4 srcPixel = read_imagef(input, sampler, (int2)(x, y));
	float p = 1.0f - srcPixel.x; // Refer to elements in a vector with x, y, z, or w components
	
	write_imagef(output, (int2)(x, y), (float4)(p, p, p, 1.0f));
}



// Gaussian filter...
// 1 2 1
// 2 4 2
// 1 2 1

kernel void blur(read_only image2d_t input, write_only image2d_t output, int w, int h)
{
	const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_REPEAT | CLK_FILTER_NEAREST;
	
	int x = get_global_id(0);
	int y = get_global_id(1);

	// Read pixels and multiply with gaussian kernel coefficients
	float p00 = read_imagef(input, sampler, (int2)(x-1, y-1)).x * (1.0f/16.0f);
	float p01 = read_imagef(input, sampler, (int2)(x, y-1)).x * (2.0f/16.0f);
	float p02 = read_imagef(input, sampler, (int2)(x+1, y-1)).x * (1.0f/16.0f);

	float p10 = read_imagef(input, sampler, (int2)(x-1, y)).x * (2.0f/16.0f);
	float p11 = read_imagef(input, sampler, (int2)(x, y)).x * (4.0f/16.0f);
	float p12 = read_imagef(input, sampler, (int2)(x+1, y)).x * (2.0f/16.0f);

	float p20 = read_imagef(input, sampler, (int2)(x-1, y+1)).x * (1.0f/16.0f);
	float p21 = read_imagef(input, sampler, (int2)(x, y+1)).x * (2.0f/16.0f);
	float p22 = read_imagef(input, sampler, (int2)(x+1, y+1)).x * (1.0f/16.0f);

	float result = p00 + p01 + p02 + p10 + p11 + p12 + p20 + p21 + p22;

	write_imagef(output, (int2)(x, y), (float4)(result));
}
