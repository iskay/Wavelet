//Various structs used throughout the program
/////////////////////////////////////////////////

#ifndef _Structs_h
#define _Structs_h

#include <string>

using namespace std;

#define HEADER_SIZE 20	//size (in bytes) of wlt header not including frequency table

//Holds info for the compressed file header
struct wlt_header_info
{
	unsigned short hsize;		//size of the wlt header
	int img_size;				//number of uncompressed image bytes
	char steps;					//number of downsample steps (either 1 or 0)
	int input_bytes;			//number of compressed image bytes
	char h_padding;				//number of padding bits at the end of the encoded image data
	double scale;				//the scaling value for the transformed coefficients
	string frequency;			//frequency table for huffman decoding
								//format: "[value1] [freq1]/[value2] [freq2]/..."
								//where any values (0 - 255) not listed are assumed
								//to have zero frequency
};

//Just a pixel colour value
struct pixel
{
	double red;
	double blue;
	double green;
};

//A node for the Huffman tree
struct hNode
{

	int value;				//the value (0 - 255) of this node if it is a
							//leaf. If it is not a leaf, this will be set
							//to (-1)
	
	unsigned int freq;		//the frequency that this symbol occurs (if it is a leaf)
	char bit;				//either 0 or 1, for generating the Huffman codewords
	char is_root;			//1 if this is a root node
	hNode *parent, *l_child, *r_child;	//pointers to connected nodes
};

#endif