#include <stdio.h>
#include <string.h>
#include "Huffman.h"

struct hNode;
struct wlt_header_info;

//Huffman encodes the input.
//"size" is the input size in bytes
//wlt is an empty header struct used to hold information that will be
//necessary for Huffman decoding later on
void HuffEncode(unsigned char *input, int size, wlt_header_info &wlt)
{
	unsigned int frequency[256];
	unsigned char c;
	int i, j;
	hNode Tree[512];

	//clear
	for (i=0; i<256; i++) frequency[i] = 0;

	//get frequencies
	for(i=0; i<size; i++)
	{
		c = input[i];
		frequency[c]++;
	}

	//initialize tree
	for(i=0; i<512; i++)
	{
		Tree[i].freq=0;
		Tree[i].bit='2';
		Tree[i].value=-1;
		Tree[i].is_root=0;
		Tree[i].parent=NULL;
		Tree[i].l_child=NULL;
		Tree[i].r_child=NULL;
	}

	//set up leaf nodes
	for(i=0; i<256; i++)
	{
		if(frequency[i] != 0)
		{
			Tree[i].freq = frequency[i];
			Tree[i].is_root = 1;
			Tree[i].value = i;
		}
	}

	int fmin1, fmin2;  //first and second smallest frequencies
	int min1, min2; //indices for smallest frequencies
	min1 = min2 = 0;
	int p=256;
	int root_count;
	//make tree
	do
	{
		root_count = 0;
		fmin1 = fmin2 = size;
		//get minimum two frequency values
		for(i=0; i<512; i++)
		{
			if(Tree[i].is_root==1)
			{
				if(Tree[i].freq < fmin1)
				{
					fmin2 = fmin1;
					min2 = min1;
					fmin1 = Tree[i].freq;
					min1 = i;
				}
				else if(Tree[i].freq < fmin2)
				{
					fmin2 = Tree[i].freq;
					min2 = i;
				}

				root_count++;
			}
		}

		Tree[p].l_child = &Tree[min1];
		Tree[p].r_child = &Tree[min2];
		Tree[p].freq = Tree[min1].freq + Tree[min2].freq;
		Tree[p].is_root = 1;

		Tree[min1].is_root = Tree[min2].is_root = 0;
		Tree[min1].parent = Tree[min2].parent = &Tree[p];
		Tree[min1].bit = '0';
		Tree[min2].bit = '1';

		p++;
		root_count--;
	
	}while(root_count != 1);

	//get the codes (backwards)
	string codes[256];
	for(i=0; i<256; i++) codes[i] = "";

	for(i=0; i<256; i++)
	{
		if(Tree[i].value != -1)
		{
			char bit = Tree[i].bit;
			hNode *tmp = &Tree[i];
			while(tmp->is_root != 1)
			{
				codes[i] += bit;
				tmp = tmp->parent;
				bit = tmp->bit;
			}
		}
	}

	//pack the frequency table into the wlt header struct
	wlt.frequency = "";
	for(i=0; i<256; i++)
	{
		if(frequency[i] != 0)
		{
			char f[16];
			sprintf(f, "%d %d/", i, frequency[i]);
			wlt.frequency += f;
		}
	}
	wlt.hsize = strlen(wlt.frequency.c_str());

	//encode
	unsigned char *output = new unsigned char[size];
	char codeword[255];
	int codelength;
	int b = 8; //bit counter
	int by= 0; //byte counter
	unsigned char bitbuf = 0; //holds bits until they're ready to be written
	for(i=0; i<size; i++)
	{
		//get value
		c = input[i];
		
		//get codeword
		strcpy(codeword, codes[c].c_str());
		codelength = strlen(codeword);

		//write bits to output
		for(j=codelength-1; j>=0; j--)
		{
			//add a bit
			bitbuf = (bitbuf << 1);
			if(codeword[j] == '1') bitbuf = (bitbuf | 1); //write a one if appropriate
			b--;
			//check if we have a full byte
			if(b==0)
			{
				output[by] = bitbuf;
				by++;
				bitbuf = 0;
				b = 8;
			}
		}
		
	}

	//pad the last byte if necessary
	wlt.h_padding = 0;
	if(b != 0 && b != 8)
	{
		for(i=0; i<b; i++)
		{
			bitbuf = (bitbuf << 1);
			wlt.h_padding++;
		}
		by++;		
		output[by] = bitbuf;
	}

	wlt.input_bytes = by+1;

	//write back to orignal array
	for(i=0; i<size; i++) input[i] = 0;
	memcpy(input, output, by+1);

	delete[] output;
}



//Decodes the input
//"size" is the input size in bytes
//wlt is a (filled) header struct that holds information necessary for
//decoding, such as the frequency table, etc.
void HuffDecode(unsigned char *input, int size, wlt_header_info wlt)
{
	unsigned int frequency[256];
	int i, s;
	hNode Tree[512];
	hNode *root;

	//unpack frequency table from wlt header struct
	for(i=0; i<256; i++) frequency[i] = 0;
	do
	{
		//parse for groups of two values, inded and freq, which are separated by "/"
		s = wlt.frequency.find("/", 0); //find first /
		if(s==string::npos) break; //stop infinite loop, but shouldn't ever be needed

		string sub = wlt.frequency.substr(0, s);
		int index, freq;
		sscanf(sub.c_str(), "%d %d", &index, &freq);
		frequency[index] = freq;
		
		wlt.frequency.erase(0, s+1);
	}while(wlt.frequency != "");

	//initialize tree
	for(i=0; i<512; i++)
	{
		Tree[i].freq=0;
		Tree[i].bit='2';
		Tree[i].value=-1;
		Tree[i].is_root=0;
		Tree[i].parent=NULL;
		Tree[i].l_child=NULL;
		Tree[i].r_child=NULL;
	}

	//set up leaf nodes
	for(i=0; i<256; i++)
	{
		if(frequency[i] != 0)
		{
			Tree[i].freq = frequency[i];
			Tree[i].is_root = 1;
			Tree[i].value = i;
		}
	}

	int fmin1, fmin2;  //first and second smallest frequencies
	int min1, min2; //indices for smallest frequencies
	min1 = min2 = 0;
	int p=256;
	int root_count;
	//make tree
	do
	{
		root_count = 0;
		fmin1 = fmin2 = size;
		//get minimum two frequency values
		for(i=0; i<512; i++)
		{
			if(Tree[i].is_root==1)
			{
				if(Tree[i].freq < fmin1)
				{
					fmin2 = fmin1;
					min2 = min1;
					fmin1 = Tree[i].freq;
					min1 = i;
				}
				else if(Tree[i].freq < fmin2)
				{
					fmin2 = Tree[i].freq;
					min2 = i;
				}

				root_count++;
			}
		}

		Tree[p].l_child = &Tree[min1];
		Tree[p].r_child = &Tree[min2];
		Tree[p].freq = Tree[min1].freq + Tree[min2].freq;
		Tree[p].is_root = 1;

		Tree[min1].is_root = Tree[min2].is_root = 0;
		Tree[min1].parent = Tree[min2].parent = &Tree[p];
		Tree[min1].bit = '0';
		Tree[min2].bit = '1';

		p++;
		root_count--;
	
	}while(root_count != 1);

	//save root node address for later
	root = Tree[min1].parent;

	//decode
	unsigned char *output = new unsigned char[size];
	int b = 8; //bit counter
	int by= 0; //byte counter
	int by_out = 0;
	unsigned char bitbuf; //holds bits we've read in
	unsigned char bit; //the next bit of the huffman sequence
	unsigned char buf; //just a temp variable
	hNode *cur = root;

	//read in a byte
	bitbuf = input[by];
	
	do
	{
		//get bits one at a time
		//start at the root and continue down until we get to a leaf
		buf = bitbuf & 0x80;
		if(buf == 0) bit = 0;
		else bit = 1;
		bitbuf = (bitbuf << 1);
		b--;

		if(bit == 0) cur = cur->l_child;
		else cur = cur->r_child;

		//check if we're at a leaf
		if(cur->value != -1)
		{
			output[by_out] = cur->value;
			by_out++;
			cur = root;
		}

		if(b==0) //read in the next byte if necessary
		{
			by++;
			bitbuf = input[by];
			b=8;
		}
	}while(by < wlt.input_bytes-2);

	//handle the last byte by itself due to padding
	b = wlt.h_padding;
	for(i=b; i>0; i--)
	{
		buf = bitbuf & 0x80;
		if(buf == 0) bit = 0;
		else bit = 1;
		bitbuf = (bitbuf << 1);

		if(bit == 0) cur = cur->l_child;
		else cur = cur->r_child;

		//check if we're at a leaf
		if(cur->value != -1)
		{
			output[by_out] = cur->value;
			cur = root;
		}
	}

	//write back to orignal array
	memcpy(input, output, by_out+1);

	delete[] output;
}