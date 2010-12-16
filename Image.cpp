#include <fstream>
#include <string.h>
#include "Image.h"
#include "Structs.h"

using namespace std;

//Loads a BMP for compression.
//fname = the name of the bmp to open
//header_addr and img_addr are used to store the address of the header and image
//arrays in memory after the file has been loaded.
//header_size and img_size are used to store the size in bytes of the header
//and image arrays.
//Currently, the input BMP must be a square with sides = 2^n, (n>1), otherwise
//the wavelet transform algorithms won't work.

int LoadBMP(char *fname, int &header_addr, int &header_size, int &img_addr, int &img_size)
{

	fstream file;
	//open with position pointer at end of file
	file.open(fname, ios::in | ios::binary);
	if(!file.is_open()) return 0;

	//get the filesize
	file.seekg(0, ios::end);
	long size = file.tellg(); 
	file.seekg(ios::beg); //back to beginning of file

	unsigned char *mem = new unsigned char[size];
	file.read((char *)mem, size);
	file.close();
	
	//check first two bytes -- should always be "BM"
	//if this is a valid bmp
	if(mem[0] != 'B' || mem[1] != 'M') return 0;

	//bytes 11-14 contain the offset to the actual image data
	int offset;
	offset = mem[10];
	offset += 256 * mem[11];
	offset += 256 * 256 * mem[12];
	offset += 256 * 256 * 256 * mem[13];
	
	//now split the bmp into header and image data
	//they will be compressed seperately
	header_size = offset;
	img_size = size-offset;

	unsigned char *header_buf = new unsigned char[header_size];
	double *img_buf = new double[img_size];

	memcpy(header_buf, mem, header_size);
	for(int i=0; i<(img_size); i++) img_buf[i] = mem[offset + i];

	header_addr = (int)header_buf;
	img_addr = (int)img_buf;

	delete[] mem;

	return 1;
}


//Saves a bmp
//fname = the file name to save as
//header_data is a pointer to the bmp header data, and header_size is its size in bytes
//img_data is a pointer to the image data, and img_size is its size in bytes
int SaveBMP(char *fname, unsigned char *header_data, int header_size, double *img_data, int img_size)
{
	//recombine the bmp file
	int file_size = header_size + img_size;
	unsigned char *mem = new unsigned char[file_size];
	unsigned char *img_buf = new unsigned char[img_size];
	for(int i=0; i<img_size; i++)
	{
		img_data[i] = (int)(img_data[i]+0.5); //round up or down
		if(img_data[i] < 0) img_data[i] = 0; //one last check for valid values
		if(img_data[i] > 255) img_data[i] = 255;
		img_buf[i] = (unsigned char)img_data[i];
	}

	memcpy(mem, header_data, header_size);
	memcpy(mem+header_size, img_buf, img_size);

	//write to disk
	fstream file;
	file.open(fname, ios::out | ios::binary);
	if(!file.is_open()) return 0;
	
	file.write((char *)mem, file_size);
	file.close();

	delete[] mem;
	delete[] img_buf;
	
	return 1;
}


//Saves a compressed file
//fname = the file name to save as
//wlt is a struct that holds the data that will be written into the file's header
//(it should already be filled out before calling this function)
//the other parameters are pointers to the bmp header and (Huffman encoded) image data
//and their sizes in bytes
int SaveWLT(char *fname, wlt_header_info wlt, unsigned char *bmp_header_data,
				int bmp_header_size, unsigned char *img_data, int img_size)
{
	//combine file data
	int file_size = wlt.hsize + bmp_header_size + img_size;
	unsigned char *mem = new unsigned char[file_size];

	//write the wlt header
	//bytes 0 - 1, wlt header size
	mem[0] = (wlt.hsize & 255);
	mem[1] = (wlt.hsize & (256*256-1)) >> 8;

	//bytes 2 - 5, number of uncompressed image bytes
	mem[2] = (wlt.img_size & 0x000000FF);
	mem[3] = (wlt.img_size & 0x0000FF00) >> 8;
	mem[4] = (wlt.img_size & 0x00FF0000) >> 16;
	mem[5] = (wlt.img_size & 0xFF000000) >> 24;

	//byte 6, # of downsample steps (1 or 0);
	mem[6] = wlt.steps;

	//bytes 7 - 10, # of encoded image bytes
	mem[7] = (wlt.input_bytes & 0x000000FF);
	mem[8] = (wlt.input_bytes & 0x0000FF00) >> 8;
	mem[9] = (wlt.input_bytes & 0x00FF0000) >> 16;
	mem[10] = (wlt.input_bytes & 0xFF000000) >> 24;

	//byte 11, # of encoding padding bits
	mem[11] = wlt.h_padding;

	//bytes 12 - 19, scaling factor for transformed coefficients
	memcpy(mem + 12, &wlt.scale, sizeof(double));

	//remaining header bytes, huffman freq table
	int f_length = wlt.hsize - HEADER_SIZE;
	for(int i=0; i<f_length; i++) mem[HEADER_SIZE+i] = wlt.frequency[i];

	//copy bmp and image data
	memcpy(mem+wlt.hsize, bmp_header_data, bmp_header_size);
	memcpy(mem+wlt.hsize+bmp_header_size, img_data, img_size);

	//write to disk
	fstream file;
	file.open(fname, ios::out | ios::binary);
	if(!file.is_open()) return 0;
	
	file.write((char *)mem, file_size);
	file.close();

	delete[] mem;
	
	return 1;
}


//Loads a compressed file
//fname = the file name
//wlt should be an empty header struct where info from the file header will be stored for later use
//bmp_header_addr is used to store the address of the bmp header after it is loaded
//img_addr is used to store the address of the (Huffman encoded) image data after it is loaded
//bmp_header_size and img_size are used to store their sizes in bytes
int LoadWLT(char *fname, wlt_header_info &wlt, int &bmp_header_addr, int &bmp_header_size,
				int &img_addr, int &img_size)
{
	fstream file;
	//open with position pointer at end of file
	file.open(fname, ios::in | ios::binary);
	if(!file.is_open()) return 0;

	//get the filesize
	file.seekg(0, ios::end);
	long size = file.tellg(); 
	file.seekg(ios::beg); //back to beginning of file

	unsigned char *mem = new unsigned char[size];
	file.read((char *)mem, size);
	file.close();
	
	//read bytes 0 - 1 to get wlt header size
	wlt.hsize = mem[0];
	wlt.hsize += 256 * mem[1];

	//read bytes 2 - 5 to get number of uncompressed image bytes
	wlt.img_size = mem[2];
	wlt.img_size += 256 * mem[3];
	wlt.img_size += 256 * 256 * mem[4];
	wlt.img_size += 256 * 256 * 256 * mem[5];

	//read byte 6 to get # of downsample steps;
	wlt.steps = mem[6];

	//read bytes 7 - 10 to get # of encoded image bytes
	wlt.input_bytes = mem[7];
	wlt.input_bytes += 256 * mem[8];
	wlt.input_bytes += 256 * 256 * mem[9];
	wlt.input_bytes += 256 * 256 * 256 * mem[10];

	//read byte 11 to get # of encoding padding bits
	wlt.h_padding = mem[11];

	//read bytes 12 - 19 to get scaling factor
	memcpy(&wlt.scale, mem+12, sizeof(double));

	//read remaining header bytes to get huffman freq table
	int f_length = wlt.hsize - HEADER_SIZE;
	wlt.frequency = "";
	for(int i=0; i<f_length; i++) wlt.frequency += mem[HEADER_SIZE+i];

	
	bmp_header_size = 54; //constant for bitmaps
	img_size = wlt.img_size;

	//split file into parts
	unsigned char *bmp_hbuf = new unsigned char[bmp_header_size];
	unsigned char *img_buf;
	if(wlt.steps != 0)
	{
		img_buf = new unsigned char[img_size*4*wlt.steps];
	}
	else
	{
		img_buf = new unsigned char[img_size];
	}

	memcpy(bmp_hbuf, mem + wlt.hsize, bmp_header_size);
	memcpy(img_buf, mem+ wlt.hsize + bmp_header_size, wlt.input_bytes);
	
	bmp_header_addr = (int)bmp_hbuf;
	img_addr = (int)img_buf;

	delete[] mem;

	return 1;
}