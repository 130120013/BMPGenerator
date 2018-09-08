#include "BMPGenerator.h"
//#include <Windows.h>

double GetValue(unsigned x, unsigned y)
{
	return 0;
}

//std::vector<std::pair<double, double>, BYTE> makeLegend(double val_min, double val_max)
//{
//	double ExtremumPointsHeight = (val_max - val_min) / 20; //5% of all heights have white colour, 5% - black
//	std::vector<std::pair<std::pair<LONG, LONG>, BYTE>> heightsColours;
//	heightsColours.emplace_back(val_min, val_min + ExtremumPointsHeight, 0); //a height range for black colour
//	heightsColours.emplace_back(val_max - ExtremumPointsHeight, val_max, 255); //a height range for white colour
//
//    if (val_max - val_min - 2 * ExtremumPointsHeight < 256)
//	{
//		double step = 254 / val_max - val_min - 2 * ExtremumPointsHeight; //TODO округлять до десятых
//		//а на сколько разделять кусочков интервал? 
//		for (auto i = val_min + ExtremumPointsHeight; i < val_max - ExtremumPointsHeight; i += step)
//		{
//			heightsColours.emplace_back(val_min, val_min + ExtremumPointsHeight, i);
//		}
//	}
//	else
//	{ 
//		double step = val_max - val_min - 2 * ExtremumPointsHeight;
//		BYTE colour = 1;
//		for (auto i = val_min + ExtremumPointsHeight; i < val_max - ExtremumPointsHeight; i += step)
//		{
//			heightsColours.emplace_back(val_min, val_min + ExtremumPointsHeight, colour);
//			colour += 1;
//		}
//	}
//
//
//}

bool generateBMP(char* name, bool fDiscardFileIfExists, LONG fWidth, LONG fHeight, double val_min, double val_max, double (*GetValue)(unsigned, unsigned))
{
	FILE *fp;

	if(fp = fopen(name, "r"))
	{
		if(!fDiscardFileIfExists)
			return false;
		fclose(fp);
	}

	fp = fopen(name, "wb");

	if(!fp)
		return false;

	BYTE bfType[2] = { 'B','M' };
	fwrite(&bfType[0], 2 * BYTESIZE, 1, fp);

	DWORD bfSize = 54 + 512 * 512 * 3;
	fwrite(&bfSize, DWORDSIZE, 1, fp);

	// bfReserved1 + bfReserved2
	DWORD reserved = 0;
	fwrite(&reserved, DWORDSIZE, 1, fp);

	DWORD bfOffBits = 14;
	fwrite(&bfOffBits, DWORDSIZE, 1, fp);

	DWORD biSize = 40;
	fwrite(&biSize, DWORDSIZE, 1, fp);

	LONG biWidth = fWidth;
	fwrite(&biWidth, LONGSIZE, 1, fp);

	LONG biHeight = fHeight;
	fwrite(&biHeight, LONGSIZE, 1, fp);

	WORD biPlanes = 1;
	fwrite(&biPlanes, WORDSIZE, 1, fp);

	WORD biBitCount = 24;
	fwrite(&biBitCount, WORDSIZE, 1, fp);

	DWORD biCompression = 0; //without compression
	fwrite(&biCompression, DWORDSIZE, 1, fp);

	DWORD biSizeImage = 0;
	fwrite(&biSizeImage, DWORDSIZE, 1, fp);

	LONG biXPelsPerMeter = 0;
	fwrite(&biXPelsPerMeter, LONGSIZE, 1, fp);

	LONG biYPelsPerMeter = 0;
	fwrite(&biYPelsPerMeter, LONGSIZE, 1, fp);

	DWORD biClrUsed = 0;
	fwrite(&biClrUsed, DWORDSIZE, 1, fp);

	DWORD biClrImportant = 0;
	fwrite(&biClrImportant, DWORDSIZE, 1, fp);

	//image
	for (int k = 0; k < 512; ++k)
	{
		for (int l = 0; l < 512; ++l)
		{
			RGBTRIPLE rgb;
			rgb.rgbBlue = 100;
			rgb.rgbGreen = rgb.rgbRed = 0;
			fwrite(&rgb.rgbBlue, 1, 1, fp);
			fwrite(&rgb.rgbGreen, 1, 1, fp);
			fwrite(&rgb.rgbRed, 1, 1, fp);
		}
	}

	fclose(fp);
	delete fp;
	return true;
}

#define RANGES 8
#define RANGE_COLORS 255
#define LAST_RANGE_COLORS 1
#define MAX_COLORS ((RANGES - 1) * RANGE_COLORS + LAST_RANGE_COLORS)

bool ValToRGB(double nVal, double nMin, double nMax, RGBTRIPLE* colour)
{
	unsigned x;
	unsigned nRange;
	double nIntVal;
	double nValRange;

	if (nMax < nMin)
		return ValToRGB(nVal, nMax, nMin, colour);
	nValRange = nMax - nMin;
	if (!nValRange) //i.e. nMax == 0xffffffffffffffff and nMin == 0
		return 0;

	x = unsigned((nVal * MAX_COLORS) / nValRange);
	if (x < MAX_COLORS - LAST_RANGE_COLORS)
	{
		nRange = (unsigned int) x / RANGE_COLORS;
		nIntVal = x % RANGE_COLORS;
	}
	else
	{
		colour->rgbBlue = colour->rgbGreen = colour->rgbRed = 0xff;
		return true;
	}
	switch (nRange)
	{
		case 0:
		{
			colour->rgbBlue = nIntVal;
			colour->rgbGreen = 0;
			colour->rgbRed = 0;
			return true;
		}
		case 1:
		{
			colour->rgbBlue = 0xff;
			colour->rgbGreen = nIntVal;
			colour->rgbRed = 0;
			return true;
		}
		case 2:
		{
			colour->rgbBlue = 0xff - nIntVal;
			colour->rgbGreen = 0xff;
			colour->rgbRed = 0;
			return true;
		}
		case 3:
		{
			colour->rgbBlue = 0;
			colour->rgbGreen = 0xff;
			colour->rgbRed = nIntVal;
			return true;
		}
		case 4:
		{
			colour->rgbBlue = 0;
			colour->rgbGreen = 0xff - nIntVal;
			colour->rgbRed = 0xff;
			return true;
		}
		case 5:
		{
			colour->rgbBlue = nIntVal;
			colour->rgbGreen = 0;
			colour->rgbRed = 0xff;
			return true;
		}
		case 6:
		{
			colour->rgbBlue = 0xff;
			colour->rgbGreen = nIntVal;
			colour->rgbRed = 0xff;
			return true;
		}
		default:
			return false;
	}
}

int main(int argc, char **argv)
{
	generateBMP("Test14.bmp", true, 512, 512, 1, 1, NULL);
	return 0;
}