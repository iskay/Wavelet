//Huffman encoder/decoder functions
////////////////////////////////////////

#ifndef _Huffman_h_
#define _Huffman_h_

#include "Structs.h"

//Huffman encode
void HuffEncode(unsigned char *input, int size, wlt_header_info &wlt);

//Huffman decode
void HuffDecode(unsigned char *input, int size, wlt_header_info wlt);

#endif