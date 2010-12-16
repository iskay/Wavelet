#include <stdio.h>
#include "Wavelet.h"

using namespace std;

int main()
{
	//A simple example of how to use the Wavelet routines
	//Note: input bmp must be a square with sides 2^n, and it must be a 24-bit bmp file
	//The settings 1 for steps and 4 for quant give a file about 10% the size of the bmp with
	//reasonable image quality on decompression.

	int choice = 4;
	int steps = 1;
	int quant = 4;
	char fname[20];
	char fname_c[20];
	int exit = 0;

	printf("Wavelet Compression Program\n\nMake a choice:");
	printf("\n1. Compress a file.\n2. Decompress a file.\n3. Exit\n");
	do
	{
		scanf("%c", &choice);
		switch(choice)
		{
		case '1':
			printf("Syntax: [input filename] [output filename] [steps = 1] [quantization = 4]\n=>");
			scanf("%s %s %d %d", fname, fname_c, &steps, &quant);
			printf("Compressing...\n");
			Compress(fname, fname_c, steps, quant);
			printf("\nDone.\n\n");
			break;

		case '2':
			printf("Syntax: [input filename] [output filename]\n=>");
			scanf("%s %s", fname_c, fname);
			printf("Decompressing...\n");
			Decompress(fname_c, fname);
			printf("\nDone.\n\n");
			break;

		case '3':
			exit = 1;
			printf("\n");
			break;

		case 10: //newline char, ie: enter key
			printf("\n1. Compress a file.\n2. Decompress a file.\n3. Exit\n");
			break;

		default:
			printf("\nInvalid choice. Choose again.\n");
			break;
		}
	}while(exit == 0);

	return 0;
}
