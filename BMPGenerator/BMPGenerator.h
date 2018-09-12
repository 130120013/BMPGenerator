#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <cmath>
#include <cstdint>

#ifndef COMMON_BMPGENERATOR_H
#define COMMON_BMPGENERATOR_H

typedef struct __tagRGBTRIPLE
{
	std::uint8_t rgbBlue;
	std::uint8_t rgbGreen;
	std::uint8_t rgbRed;
} RGBTRIPLE;

#define RANGES 8
#define RANGE_COLORS 255
#define LAST_RANGE_COLORS 1
#define MAX_COLORS ((RANGES - 1) * RANGE_COLORS + LAST_RANGE_COLORS)

#define LONGSIZE sizeof(std::int32_t) //4
#define BYTESIZE sizeof(std::uint8_t) //1
#define DWORDSIZE sizeof(std::uint32_t) //4
#define WORDSIZE sizeof(std::uint16_t) //2

inline unsigned scaleBetween(double unscaledNum, double min, double max) 
{
	return unsigned(MAX_COLORS * (unscaledNum - min) / (max - min)); //scale to [0; MAX_COLORS)
}

bool ValToRGB(double nVal, double nMin, double nMax, RGBTRIPLE* colour);
FILE* CreateBitmapFile(FILE *fp, std::int32_t fWidth, std::int32_t fHeight, bool fDiscardFileIfExists);

template <class Callable>
bool generateBMP(char* name, double val_min, double val_max, Callable&& GetValue, std::int32_t fWidth, std::int32_t fHeight, bool fDiscardFileIfExists)
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

	std::uint8_t bfType[2] = { 'B','M' };
	fwrite(&bfType[0], 2 * BYTESIZE, 1, fp);

	auto cbPadding = std::int32_t(4 - (fWidth & 3));
	if (cbPadding == 4)
		cbPadding = 0;
	auto cbPaddedWidth = fWidth * 3 + cbPadding;
	std::uint32_t bfSize = 54 + cbPaddedWidth * fHeight;
	fwrite(&bfSize, DWORDSIZE, 1, fp);

	// bfReserved1 + bfReserved2
	std::uint32_t reserved = 0;
	fwrite(&reserved, DWORDSIZE, 1, fp);

	std::uint32_t bfOffBits = 14;
	fwrite(&bfOffBits, DWORDSIZE, 1, fp);

	std::uint32_t biSize = 40;
	fwrite(&biSize, DWORDSIZE, 1, fp);

	std::int32_t biWidth = fWidth;
	fwrite(&biWidth, LONGSIZE, 1, fp);

	std::int32_t biHeight = fHeight;
	fwrite(&biHeight, LONGSIZE, 1, fp);

	std::uint16_t biPlanes = 1;
	fwrite(&biPlanes, WORDSIZE, 1, fp);

	std::uint16_t biBitCount = 24;
	fwrite(&biBitCount, WORDSIZE, 1, fp);

	std::uint32_t biCompression = 0; //without compression
	fwrite(&biCompression, DWORDSIZE, 1, fp);

	std::uint32_t biSizeImage = 0;
	fwrite(&biSizeImage, DWORDSIZE, 1, fp);

	std::int32_t biXPelsPerMeter = 0;
	fwrite(&biXPelsPerMeter, LONGSIZE, 1, fp);

	std::int32_t biYPelsPerMeter = 0;
	fwrite(&biYPelsPerMeter, LONGSIZE, 1, fp);

	std::uint32_t biClrUsed = 0;
	fwrite(&biClrUsed, DWORDSIZE, 1, fp);

	std::uint32_t biClrImportant = 0;
	fwrite(&biClrImportant, DWORDSIZE, 1, fp);

	static const std::uint32_t padding = 0;

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(GetValue(k, l), val_min, val_max, &rgb);
			if(successCode) //but if successCode is false?
			{
				fwrite(&rgb.rgbBlue, 1, 1, fp);
				fwrite(&rgb.rgbGreen, 1, 1, fp);
				fwrite(&rgb.rgbRed, 1, 1, fp);
			}
		}
		fwrite(&padding, 1, cbPadding, fp);

	}

	fclose(fp);
	delete fp;
	return true;
}

template <class Callable>
bool generateBMP(char* name, Callable&& GetValue, std::int32_t fWidth, std::int32_t fHeight, bool fDiscardFileIfExists);

#endif
