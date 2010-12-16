#include <stdio.h>
#include "Wavelet.h"

//fname = name of the bmp file to compress
//fname_c = name to save the compressed file as
//steps = the number of downsampling steps, see Operations.h for more info
//quant = the quantization amount, see Operations.h for more info
void Compress(char *fname, char *fname_c, int steps, int quant)
{
	double *img = NULL;
	unsigned char *header = NULL;
	int sz_header = 0;
	int sz_img = 0;
	int h_addr = 0;
	int i_addr = 0;

	//load bmp
	if(!LoadBMP(fname, h_addr, sz_header, i_addr, sz_img))
	{
		printf("Error opening bmp file\n\n");
		return;
	}

	header = (unsigned char *)h_addr;
	img = (double *)i_addr;

	//convert to YUV colourspace
	ToYUV(img, sz_img);

	//wavelet transform
	Transform97(img, sz_img);

	wlt_header_info wlt;
	//quantize (reduce # of colours) and scale coefficients
	unsigned char *q_img = Quantize(img, sz_img, quant, wlt);
	delete[] img;

	//Downsample the resolution of the sub-bands
	Downsample(q_img, sz_img, steps);

	//huffman encode
	HuffEncode(q_img, sz_img, wlt);

	//write rest of wlt header info
	wlt.hsize += HEADER_SIZE; //size of wlt header not including frequency table
	wlt.steps = (char) steps;
	wlt.img_size = sz_img;

	//save a .wlt file
	if(!SaveWLT(fname_c, wlt, header, sz_header, q_img, wlt.input_bytes)) printf("Error saving wlt file\n\n");

	//cleanup
	delete[] q_img;
	delete[] header;
}


//fname_c = the name of the compressed file to decompress
//fname = the name to save the resulting bmp under
void Decompress(char *fname_c, char *fname)
{
	//open a .wlt file
	unsigned char *img = NULL;
	unsigned char *header = NULL;
	int sz_header = 0;
	int sz_img = 0;
	int h_addr = 0;
	int i_addr = 0;

	wlt_header_info wlt;
	if(!LoadWLT(fname_c, wlt, h_addr, sz_header, i_addr, sz_img))
	{
		printf("Error loading compressed file\n\n");
		return;
	}

	header = (unsigned char *)h_addr;
	img = (unsigned char *)i_addr;

	//huffman decode
	HuffDecode(img, sz_img, wlt);

	//scale back up to original size
	Upsample(img, sz_img, wlt.steps);

	//put image data back into a double array before inversing to avoid rounding errors
	double *img2 = new double[sz_img];
	for(int i=0; i<sz_img; i++)
	{
		img2[i] = (double)img[i];
	}
	delete[] img;

	//undo the scaling of coefficients
	Rescale(img2, sz_img, wlt);

	//inverse wavelet transform
	Inverse97(img2, sz_img, wlt.steps);

	//convert back to RGB colourspace
	ToRGB(img2, sz_img);

	//save result to disk
	if(!SaveBMP(fname, header, sz_header, img2, sz_img)) printf("Error saving bmp file\n\n");

	//cleanup
	delete[] img2;
	delete[] header;
}