//Main wavelet functions
/////////////////////////////

#ifndef _Wavelet_h
#define _Wavelet_h

#include "Structs.h"
#include "Image.h"
#include "Operations.h"
#include "Huffman.h"

//Compress a bmp file
void Compress(char *fname, char *fname_c, int steps = 1, int quant = 16);

//Decompress to a bmp file
void Decompress(char *fname, char *fname_c);

#endif