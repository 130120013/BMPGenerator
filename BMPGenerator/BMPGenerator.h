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
FILE* CreateBitmapFile(char* name, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists, int32_t* cbPadding);

template <class Callable>
bool generateBMP(char* name, double val_min, double val_max, Callable&& GetValue, std::int32_t fWidth, std::int32_t fHeight, bool fDiscardFileIfExists)
{
	std::int32_t cbPadding; 
	FILE* fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

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

template <class Iterator>
auto Min(Iterator arrBegin, std::int32_t fWidth, std::int32_t fHeight) //todo
{
	auto tempMin = arrBegin[0][0];
	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			if (arrBegin[k][l] < tempMin)
				tempMin = arrBegin[k][l];
		}
	}
	return tempMin;
}

template <class Iterator>
auto Max(Iterator arrBegin, std::int32_t fWidth, std::int32_t fHeight) //todo
{
	auto tempMax = arrBegin[0][0];
	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			if (arrBegin[k][l] > tempMax)
				tempMax = arrBegin[k][l];
		}
	}
	return tempMax;
}

template <class Callable>
bool generateBMP(char* name, Callable&& GetValue, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists)
{
	std::int32_t cbPadding;
	FILE* fp = CreateBitmapFile(name, fWidth, fHeight, fDiscardFileIfExists, &cbPadding);
	if (!fp)
		return false;

	static const std::uint32_t padding = 0;
	double** heightMatrix = new double*[fWidth];
	for (int i = 0; i < fWidth; ++i)
		heightMatrix[i] = new double[fHeight];

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			heightMatrix[k][l] = GetValue(k, l);
		}
	}
	auto minVal = Min(heightMatrix, fWidth, fHeight);
	auto maxVal = Max(heightMatrix, fWidth, fHeight);

	for (int l = 0; l < fHeight; ++l)
	{
		for (int k = 0; k < fWidth; ++k)
		{
			RGBTRIPLE rgb;
			bool successCode = ValToRGB(GetValue(k, l), minVal, maxVal, &rgb);
			if (successCode) //but if successCode is false?
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
#endif
