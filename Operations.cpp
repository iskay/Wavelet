#include "Operations.h"
#include "Structs.h"
#include <string.h>
#include <math.h>

using namespace std;

#define MIN_SUBI	128	//when the subimages reach this size, we will stop our transforms

//Downsamples the x and y resolution of the LH, HL and HH subbands by 2x each. Since most of
//the important info is in the LL sub-band, we can save quite a lot of space with only a small
//drop in quality. After this step we will have less than half the original # of pixels to send.
//The new image size will be stored in img_size and "steps" is the number of sub-band levels to
//downsample (currently you can only do the first level sub-bands, so steps must be 1 or 0).
void Downsample(unsigned char *input, int &img_size, int steps)
{
	if(steps==0) return;  //user can choose no downsampling if they wish
	//w is the length of one side in pixels
	int w = sqrt((float)(img_size/3));
	//there are only so many levels of sub-bands, determined by the image size and MIN_SUBI
	int max_steps = log((float)(w/MIN_SUBI))/log((float)2);
	int row_length = 3 * w; //because each pixel consists of three bytes
	
	//a constant for converting from 2-d array co-ordinate to 1-d array co-ordinates
	//input[i][j] = input[i*array_length + j] (it's easier to visualize in 2-d)
	int array_length = row_length;

	if(steps > max_steps) steps = max_steps;
	int sub_size = img_size / 4;

	//right now, you can only downsample the first set of subbands
	//in the future this feature should be improved
	if(steps > 1) steps = 1;

	//a buffer to hold our changes
	unsigned char *buffer = new unsigned char[(int)(1.75*sub_size)];

	for(int n=steps; n>0; n--)
	{
		//buffer for holding a subimage
		unsigned char *sub = new unsigned char[sub_size];

		//buffer for holding the downsampled subimage data
		unsigned char *dsamp = new unsigned char[(int)(0.75 * sub_size)];

		//grab top-left subimage (we don't want to change it)
		int i, j;
		pixel tl, tr, bl, br, avg;
		for(i=0; i<w/2; i++)
		{
			for(j=0; j<row_length/2; j++)
			{
				sub[i*(row_length/2) +j] = input[i*array_length +j];
			}
		}

		//grab 2x2 groups of pixels and average them
		int a=0;
		for(i=0; i<w; i=i+2)
		{
			for(j=0; j<row_length; j=j+6)
			{
				//ignore data from top-left subimage
				if(i < w/2 && j < row_length/2) continue;
				tl.blue = input[i*row_length +j];
				tl.green = input[i*row_length +j+1];
				tl.red = input[i*row_length +j+2];

				tr.blue = input[i*row_length +j+3];
				tr.green = input[i*row_length +j+4];
				tr.red = input[i*row_length +j+5];

				bl.blue = input[(i+1)*row_length +j];
				bl.green = input[(i+1)*row_length +j+1];
				bl.red = input[(i+1)*row_length +j+2];

				br.blue = input[(i+1)*row_length +j+3];
				br.green = input[(i+1)*row_length +j+4];
				br.red = input[(i+1)*row_length +j+5];

				avg.blue = (tr.blue + tl.blue + br.blue + bl.blue) / 4;
				avg.green = (tr.green + tl.green + br.green + bl.green) / 4;
				avg.red = (tr.red + tl.red + br.red + bl.red) / 4;

				dsamp[a] = avg.blue;
				dsamp[a+1] = avg.green;
				dsamp[a+2] = avg.red;
				a=a+3;				
			}
		}
		//copy top-left subimage to our buffer
		memcpy(buffer, sub, sub_size);

		//copy the averaged data to our buffer
		memcpy(buffer+sub_size, dsamp, (int)(0.75*sub_size));
		/*
		w = w/2;
		row_length = row_length / 2;
		sub_size = sub_size / 4;
		*/
		delete[] sub;
		delete[] dsamp;
	}
	//clear input array
	for(int i=0; i<img_size; i++) input[i]=0;

	//update img_size - one full subimage + 1/4 each of the others, and each
	//subimage is 1/4 the image size
	img_size = 1.75 * (img_size/4);

	//copy our data
	memcpy(input, buffer, img_size);
	delete[] buffer;
}


//Rescales everything to the original size, and puts the updated image size in bytes
//in img_size. The value of steps should be the same as the number of
//downsample steps applied.
void Upsample(unsigned char *input, int &img_size, int steps)
{
	if(steps==0) return;

	img_size = img_size / 1.75 * 4;
	int w = sqrt((float)(img_size/3));
	int row_length = 3 * w; //because each pixel consists of three bytes

	//unpack -- reverse the order we packed in the array
	int sub_size = img_size / 4;
	unsigned char *sub = new unsigned char[sub_size];
	unsigned char *dsamp = new unsigned char[(int)(0.75 * sub_size)];
	memcpy(sub, input, sub_size);
	memcpy(dsamp, input+sub_size, (int)(0.75 * sub_size));
	int i, j;
	int a=0;
		
	//refill the 2x2 blocks with the average
	for(i=0; i<w; i=i+2)
	{
		for(j=0; j<row_length; j=j+6)
		{
			if(i < w/2 && j < row_length/2) continue;  //skip any top-left pixels
			input[i*row_length +j]  = dsamp[a]; //blue
			input[i*row_length +j+1]  = dsamp[a+1]; //green
			input[i*row_length +j+2]  = dsamp[a+2]; //red

			input[i*row_length +j+3] = dsamp[a];
			input[i*row_length +j+4] = dsamp[a+1];
			input[i*row_length +j+5] = dsamp[a+2];

			input[(i+1)*row_length +j]  = dsamp[a];
			input[(i+1)*row_length +j+1]  = dsamp[a+1];
			input[(i+1)*row_length +j+2]  = dsamp[a+2];

			input[(i+1)*row_length +j+3] = dsamp[a];
			input[(i+1)*row_length +j+4] = dsamp[a+1];
			input[(i+1)*row_length +j+5] = dsamp[a+2];
			a=a+3;
		}
	}
	//copy top-left subimage back
	for(i=0; i<w/2; i++)
	{
		for(j=0; j<row_length/2; j++)
		{
			input[i*row_length + j] = sub[i*(row_length/2) +j];
		}
	}
	delete[] sub;
	delete[] dsamp;
}


//First, scales all coefficients so they will fit in an 8-bit char. After the transform
//they could be very large +ve or -ve values, and they need to fit between 0 - 255.
//Then, quantizes the pixel values according to the value of amount; ie: reduces the number
//of colours. This leads to better Huffman encoding, so there is a tradeoff between quality
//and compression. Values between 1 and 8 work well for most images.
unsigned char *Quantize(double *input, int img_size, int amount, wlt_header_info &wlt)
{
	if(amount > 64) amount == 64;
	unsigned char *q = new unsigned char[img_size];
	int i;

	//first we need to adjust all coefficients to make sure they are between 0 - 255
	//we will scale them according to the maximum value and then center them around 128
	double max=0;
	int do_scale = 0;
	for(i=0; i<img_size; i++)
	{
		int abs;
		if(input[i] < 0) abs = input[i] * (-1);
		else abs = input[i];
		if(abs > max) max = abs;
	}
	if(max > 255) do_scale = 1;
	double scale = max / 128;
	wlt.scale = scale; //we will need this value on decompression

	for(i=0; i<img_size; i++)
	{
		//scale to between 0 - 255
		if(do_scale == 1) input[i] = (input[i] / scale) + 128;
		//now, quantize the pixel values
		input[i] = input[i] / amount;
		input[i] = (int)(input[i]+0.5); //round up or down
		input[i] = input[i] * amount; //scale back up
		if(input[i] > 255) input[i]=255;
		if(input[i] < 0) input[i]=0;
		q[i] = ((unsigned char)input[i]);
	}
	return q;
}


//Rescales all coefficients to their original value after Quantize(...)
void Rescale(double *input, int img_size, wlt_header_info wlt)
{
	for(int i=0; i<img_size; i++)
	{
		input[i] = (input[i] - 128) * wlt.scale;
	}
}


//Convert from RGB to YUV, where img_size is the size of the input in bytes.
void ToYUV(double *input,int img_size)
{
	//Transorm defined as:
	//|Y| = |1/4    1/2     1/4| |R|
	//|U| = |1      -1      0  | |G|
	//|V| = |0      -1      1  | |B|
	double *output = new double[img_size];
	int i;
	pixel rgb;

	for(i=0; i<img_size; i=i+3)
	{
		rgb.blue = input[i];
		rgb.green = input[i+1];
		rgb.red = input[i+2];
		
		output[i] = -1 * rgb.green + rgb.blue;  //V
		output[i+1] = rgb.red - rgb.green;  //U
		output[i+2] = 0.25 * rgb.red + 0.5 * rgb.green + 0.25 * rgb.blue;  //Y
	}
	for(i=0; i<img_size; i++) input[i] = output[i];
	delete[] output;
}


//Convert from YUV to RGB, where img_size is the size of the input in bytes.
void ToRGB(double *input,int img_size)
{
	//Transform defined as:
	//|R| = |1      3/4    -1/4| |Y|
	//|G| = |1     -1/4    -1/4| |U|
	//|B| = |1     -1/4     3/4| |V|
	double *output = new double[img_size];
	int i;
	pixel yuv;

	for(i=0; i<img_size; i=i+3)
	{
		yuv.blue = input[i];		//V
		yuv.green = input[i+1];		//U
		yuv.red = input[i+2];		//Y
		
		output[i] = yuv.red - 0.25 * yuv.green + 0.75 * yuv.blue;   //B	
		output[i+1] = yuv.red - 0.25 * yuv.green - 0.25 * yuv.blue;  //G	
		output[i+2] = yuv.red  + 0.75 * yuv.green - 0.25 * yuv.blue;  //R
	}
	for(i=0; i<img_size; i++) input[i] = output[i];
	delete[] output;
}


//Performs a Daubechies 9/7 wavelet transform on the input image, where img_size is
//the size of the input in bytes.
void Transform97(double *input, int img_size)
{
	//w is the length of one side in pixels
	int w = sqrt((float)(img_size/3));	
	int row_length = 3 * w;
	int i, j, a;
	double *Red = new double[img_size/3];
	double *Green = new double[img_size/3];
	double *Blue = new double[img_size/3];

	//split image data into red, blue, green streams
	for(i=0; i<w; i++)
	{
		a=0;
		for(j=0; j<row_length; j=j+3)
		{
			Blue[i*w + a] = input[i*row_length + j];
			Green[i*w + a] = input[i*row_length + j+1];
			Red[i*w + a] = input[i*row_length + j+2];
			a++;
		}
	}
	//transform each separately
	TransformStream(Red, img_size/3);
	TransformStream(Green, img_size/3);
	TransformStream(Blue, img_size/3);

	//recombine streams
	for(i=0; i<w; i++)
	{
		a=0;
		for(j=0; j<row_length; j=j+3)
		{
			input[i*row_length + j] = Blue[i*w + a];
			input[i*row_length + j+1] = Green[i*w + a];
			input[i*row_length + j+2] = Red[i*w + a];
			a++;
		}
	}
	delete[] Red;
	delete[] Green;
	delete[] Blue;
}

//Transforms a single channel (red, blue, green) of the image
void TransformStream(double *input, int img_size)
{
	int w = sqrt((float)(img_size));
	//set a limit on how small the smallest subimage can be
	//we call this function recursively until it reaches this limit
	if(w == MIN_SUBI) return;
	int i, j;
	//temp array for the transform
	double *vector = new double[w];

	//apply transform to rows
	for(i=0; i<w; i++)
	{
		//get row
		for(j=0; j<w; j++)
		{
			vector[j] = input[i*w + j];
		}
		//apply transform
		Step97(vector, w);
		//copy back
		for(j=0; j<w; j++)
		{
			input[i*w + j] = vector[j];
		}
	}
	//now apply transform to the columns of our new array
	for(j=0; j<w; j++)
	{
		//get column
		for(i=0; i<w; i++)
		{
			vector[i] = input[i*w + j];
		}
		//apply transform
		Step97(vector, w);
		//copy back
		for(i=0; i<w; i++)
		{
			input[i*w + j] = vector[i];
		}
	}
	//copy top-left subimage into a new array and repeat
	double *subimage = new double[img_size/4];
	for(i=0; i<w/2; i++)
	{
		for(j=0; j<w/2; j++)
		{
			subimage[i*(w/2) + j] = input[i*w + j];
		}
	}
	TransformStream(subimage, img_size/4);

	//copy new subimage back to top left corner
	for(i=0; i<w/2; i++)
	{
		for(j=0; j<w/2; j++)
		{
			input[i*w + j] = subimage[i*(w/2) + j];
		}
	}
	delete[] vector;
}

//Applies the filters to the input (as a 1-d array). Used on one row or column
//of image data at a time. Taken from a sample algorithm at http://www.ebi.ac.uk/~gpau/misc/dwt97.c
void Step97(double* input, int img_size)
{
	double a;
	int i;

	// Predict 1
	a=-1.586134342;
	for (i=1;i<img_size-2;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	} 
	input[img_size-1]+=2*a*input[img_size-2];

	// Update 1
	a=-0.05298011854;
	for (i=2;i<img_size;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[0]+=2*a*input[1];

	// Predict 2
	a=0.8829110762;
	for (i=1;i<img_size-2;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[img_size-1]+=2*a*input[img_size-2];

	// Update 2
	a=0.4435068522;
	for (i=2;i<img_size;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[0]+=2*a*input[1];

	// Scale
	a=1/1.149604398;
	for (i=0;i<img_size;i++)
	{
		if (i%2) input[i]*=a;
		else input[i]/=a;
	}

	// Pack
	double *temp = new double[img_size];
	for (i=0;i<img_size;i++)
	{
		if (i%2==0) temp[i/2]=input[i];
		else temp[img_size/2+i/2]=input[i];
	}
	for (i=0;i<img_size;i++) input[i]=temp[i];
	delete[] temp;
}

//Performs a Daubechies 9/7 Inverse transfom
void Inverse97(double *input, int img_size, int steps)
{
	int w = sqrt((float)(img_size/3));
	int row_length = 3 * w;
	int i, j, a;
	double *Red = new double[img_size/3];
	double *Green = new double[img_size/3];
	double *Blue = new double[img_size/3];

	//split image data into red, blue, green streams
	for(i=0; i<w; i++)
	{
		a=0;
		for(j=0; j<row_length; j=j+3)
		{
			Blue[i*w + a] = input[i*row_length + j];
			Green[i*w + a] = input[i*row_length + j+1];
			Red[i*w + a] = input[i*row_length + j+2];
			a++;
		}
	}
	//transform each separately
	InverseStream(Red, img_size/3, steps);
	InverseStream(Green, img_size/3, steps);
	InverseStream(Blue, img_size/3, steps);

	//recombine streams
	for(i=0; i<w; i++)
	{
		a=0;
		for(j=0; j<row_length; j=j+3)
		{
			input[i*row_length + j] = Blue[i*w + a];
			input[i*row_length + j+1] = Green[i*w + a];
			input[i*row_length + j+2] = Red[i*w + a];
			a++;
		}
	}
	delete[] Red;
	delete[] Green;
	delete[] Blue;
}

//Performs an Inverse on a single stream (red, blue, or green)
//"steps" is the number of downsample steps performed during compression
void InverseStream(double *input, int img_size, int steps)
{
	//w_orig is the length of one side of the original image in pixels
	//w_sub should initially be double the length of one side of the smallest subimage
	int w_sub = 2 * MIN_SUBI;// * pow((float)2, steps);
	int w_orig = sqrt((float)(img_size));
	int i, j;
	//number of inverses needed
	int n = log((float)(w_orig/w_sub))/log((float)2);
	int row_length = w_orig;

	do
	{
		//temp array for the inverse
		double *vector = new double[w_sub];
		double *subimage = new double[w_sub*w_sub];

		//get the top-left subimages to transform
		for(i=0; i<w_sub; i++)
		{
			for(j=0; j<w_sub; j++)
			{
				subimage[i*w_sub + j] = input[i*row_length + j];
			}
		}

		//first apply inverse to the columns of our new array
		for(j=0; j<w_sub; j++)
		{
			//get column
			for(i=0; i<w_sub; i++)
			{
				vector[i] = subimage[i*w_sub + j];
			}
			//apply inverse
			InvStep97(vector, w_sub);
			//copy back
			for(i=0; i<w_sub; i++)
			{
				subimage[i*w_sub + j] = vector[i];
			}
		}
		//now apply inverse to rows
		for(i=0; i<w_sub; i++)
		{
			//get row
			for(j=0; j<w_sub; j++)
			{
				vector[j] = subimage[i*w_sub + j];
			}
			//apply inverse
			InvStep97(vector, w_sub);
			//copy back
			for(j=0; j<w_sub; j++)
			{
				subimage[i*w_sub + j] = vector[j];
			}
		}
		//copy the inversed subimage back to the original image and repeat
		for(i=0; i<w_sub; i++)
		{
			for(j=0; j<w_sub; j++)
			{
				input[i*row_length + j] = subimage[i*w_sub + j];
			}
		}
		delete[] subimage;
		delete[] vector;
		n--;
		w_sub = w_sub * 2;
	}while(n != -1);
}

//Performs a filter inverse on a one-dimensional array. Use on a row or column of
//image data. Taken from a sample algorithm at http://www.ebi.ac.uk/~gpau/misc/dwt97.c
void InvStep97(double* input, int img_size)
{
	double a;
	int i;

	// Unpack
	double *temp = new double[img_size];
	for (i=0;i<img_size/2;i++)
	{
		temp[i*2]=input[i];
		temp[i*2+1]=input[i+img_size/2];
	}
	for (i=0;i<img_size;i++) input[i]=temp[i];

	// Undo scale
	a=1.149604398;
	for (i=0;i<img_size;i++)
	{
		if (i%2) input[i]*=a;
		else input[i]/=a;
	}

	// Undo update 2
	a=-0.4435068522;
	for (i=2;i<img_size;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[0]+=2*a*input[1];

	// Undo predict 2
	a=-0.8829110762;
	for (i=1;i<img_size-2;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[img_size-1]+=2*a*input[img_size-2];

	// Undo update 1
	a=0.05298011854;
	for (i=2;i<img_size;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	}
	input[0]+=2*a*input[1];

	// Undo predict 1
	a=1.586134342;
	for (i=1;i<img_size-2;i+=2)
	{
		input[i]+=a*(input[i-1]+input[i+1]);
	} 
	input[img_size-1]+=2*a*input[img_size-2];
}