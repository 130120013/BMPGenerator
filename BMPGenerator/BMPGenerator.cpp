#include "BMPGenerator.h"

#define RANGES 8
#define RANGE_COLORS 255
#define LAST_RANGE_COLORS 1
#define MAX_COLORS ((RANGES - 1) * RANGE_COLORS + LAST_RANGE_COLORS)

double GetValue(unsigned x = 0, unsigned y = 0)
{
	return 0;
}

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
		nRange = (unsigned int)x / RANGE_COLORS;
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

bool generateBMP(char* name, bool fDiscardFileIfExists, LONG fWidth, LONG fHeight, double val_min, double val_max, double (*GetValue)(unsigned, unsigned) )
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

	DWORD bfSize = 54 + fWidth * fHeight * 3;
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
	std::vector<std::vector<int>> heights;
	srand(std::time(0));

	for (int k = 0; k < fWidth; ++k)
	{
		std::vector<int> temp;
		for (int l = 0; l < fHeight; ++l)
		{
			temp.emplace_back((int)val_min + rand() % (int)val_max);
		}
		heights.emplace_back(temp);
	}

	for (int k = 0; k < fWidth; ++k)
	{
		for (int l = 0; l < fHeight; ++l)
		{
			RGBTRIPLE rgb;
			ValToRGB(heights[k][l], val_min, val_max, &rgb);
			fwrite(&rgb.rgbBlue, 1, 1, fp);
			fwrite(&rgb.rgbGreen, 1, 1, fp);
			fwrite(&rgb.rgbRed, 1, 1, fp);
		}
	}

	fclose(fp);
	delete fp;
	return true;
}

int main(int argc, char **argv)
{
	double(*GetVal)(unsigned, unsigned);
	GetVal = &GetValue;
	generateBMP("Test15.bmp", true, 1500, 200, 12, 350, GetVal);
	system("pause");
	return 0;
}