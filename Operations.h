//Wavelet operations
///////////////////////

#ifndef _Operations_h
#define _Operations_h

#include "Structs.h"

//Downsamples the resolution of the sub-bands (lossy compression)
void Downsample(unsigned char *input, int &img_size, int steps);

//Rescales everything back to original size
void Upsample(unsigned char *input, int &img_size, int steps);

//Quantizes the pixel values according to (amount) and scales them so they are all
//between 0 - 255
unsigned char *Quantize(double *input, int img_size, int amount, wlt_header_info &wlt);

//Rescales the coefficients to their original values after Quantize(...)
void Rescale(double *input, int img_size, wlt_header_info wlt);

//Converts from rgb -> yuv colourspace
void ToYUV(double *input, int img_size);

//Converts from yuv -> rgb colourspace
void ToRGB(double *input, int img_size);

//These three functions together perform a Daubechies 9/7 transform
//Transform97() is the controlling function
void Transform97(double *input, int img_size);
void TransformStream(double *input, int img_size);
void Step97(double *input, int img_size);

//These three functions together perform a Daubechies 9/7 inverse transform
//Inverse97() is the controlling function
void Inverse97(double *input, int img_size, int steps);
void InverseStream(double *input, int img_size, int steps);
void InvStep97(double* input, int img_size);

#endif