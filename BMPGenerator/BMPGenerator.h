#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility>
#include <vector>
#include <cmath> 
#include <ctime>

#ifndef COMMON_BMPGENERATOR_H
#define COMMON_BMPGENERATOR_H

typedef long LONG;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;

#define LONGSIZE sizeof(LONG) //4
#define BYTESIZE sizeof(BYTE) //1
#define DWORDSIZE sizeof(DWORD) //4
#define WORDSIZE sizeof(WORD) //2

typedef struct __tagBITMAPFILEHEADER
{
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

typedef struct __tagBITMAPINFOHEADER
{
	DWORD  biSize;
	LONG   biWidth;
	LONG   biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
} BITMAPINFOHEADER;

typedef struct __tagRGBTRIPLE
{
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
//	BYTE    rgbReserved;
} RGBTRIPLE;

typedef struct image
{
	BITMAPFILEHEADER* file_info;
	BITMAPINFOHEADER* bitmap_info;
	RGBTRIPLE *bitmap_palette;
	BYTE *data;
} IMAGE;

double GetValue(unsigned x, unsigned y);
bool ValToRGB(double nVal, double nMin, double nMax, RGBTRIPLE* colour);
bool generateBMP(char* name, bool fDiscardFileIfExists, LONG fWidth, LONG fHeight, double val_min, double val_max, double (*GetValue)(unsigned, unsigned));

#endif
