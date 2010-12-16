***********************************
**    Simple wavelet library     **
** by Ian Kay, ikay@lakeheadu.ca **
**                               **
***********************************

This is a simple wavelet-based image compression/decompression library. Currently, it only supports windows bitmap format.

The file main.cpp is a short demo program which takes a bitmap as input and produces a wavelet compressed file. Alternatively, it can take a compressed file and produce a bitmap. Of course, since wavelet compression is lossy, the new bitmap will not be identical to the old.

The file lenna.bmp is included for convenient testing.

Please feel free to use this code however you wish.