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

inline unsigned scaleBetween(const double unscaledNum, const double min, const double max) 
{
	return unsigned(MAX_COLORS * (unscaledNum - min) / (max - min)); //scale from [min; max) to [0; MAX_COLORS)
}

bool ValToRGB(const double nVal, const double nMin, const double nMax, RGBTRIPLE* colour); //converts a value to a RGB colour
FILE* CreateBitmapFile(const char* name, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists, int32_t* cbPadding); //creates a file with BMP headers

template <class Callable>
bool generateBMP(const char* name, const double val_min, const double val_max, const Callable&& GetValue, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists) 
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
auto maxTask(Iterator arrBegin, const std::int32_t fWidth, const std::int32_t fHeight) //todo
{
	using value_type = typename std::iterator_traits<Iterator>::value_type;
	constexpr auto PARALLEL_THRESHOLD = 1000;
	int threadCount = std::thread::hardware_concurrency();
	if (arrEnd - arrBegin < PARALLEL_THRESHOLD || threadCount < 2)
		return Max<Iterator>(arrBegin, arrEnd);
	std::vector<std::future<value_type>> futures;
	futures.reserve(threadCount);
	int ost = fWidth * fHeight % threadCount;
	int part = fWidth * fHeight / threadCount;
	
	while (i < arrBegin)
	{
		if (ost != 0) 
			part += 1;
		futures.emplace_back(std::async(std::launch::async, Max<Iterator>, i, i + N / threadCount));
	}

	for (auto i = arrBegin; i < arrEnd; i += (N / threadCount))
	{
		futures.emplace_back(std::async(std::launch::async, Max<Iterator>, i, i + N / threadCount));
	}

	std::vector<value_type> results;
	results.reserve(futures.size());
	for (auto& fut : futures)
		results.emplace_back(fut.get());
	return maxTask(results.begin(), results.end());
}

template <class Iterator>
auto minTask(Iterator arrBegin, Iterator arrEnd) //todo 
{
	using value_type = typename std::iterator_traits<Iterator>::value_type;
	constexpr auto PARALLEL_THRESHOLD = 1000;
	int threadCount = std::thread::hardware_concurrency();
	if (arrEnd - arrBegin < PARALLEL_THRESHOLD || threadCount < 2)
		return Min<Iterator>(arrBegin, arrEnd);
	std::vector<std::future<value_type>> futures;
	futures.reserve(threadCount);
	for (auto i = arrBegin; i < arrEnd; i += (N / threadCount))
		futures.emplace_back(std::async(std::launch::async, Min<Iterator>, i, i + N / threadCount));

	std::vector<value_type> results;
	results.reserve(futures.size());
	for (auto& fut : futures)
		results.emplace_back(fut.get());
	return minTask(results.begin(), results.end());
}


template <class Iterator>
auto Min(Iterator arrBegin, const std::size_t offset, const std::int32_t countElements, const std::int32_t fWidth) //надо передать указатель на начальную строку в матрице, offset, количество элементов для обработки, ширину матрицы
{
	auto tempMin = *(arrBegin + offset);
	auto 
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
auto Max(Iterator arrBegin, const std::int32_t fWidth, const std::int32_t fHeight)
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
bool generateBMP(const char* name, const Callable&& GetValue, const std::int32_t fWidth, const std::int32_t fHeight, const bool fDiscardFileIfExists)
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
