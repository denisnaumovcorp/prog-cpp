#ifndef CONVERTERUTILS_H
#define CONVERTERUTILS_H

#include "converterStructures.h"

void executeError(std::ifstream* inputFile, std::ofstream* outputFile, const std::string& errorText);
void executeError(std::ifstream* file1, std::ifstream* file2, const std::string& errorText);

void initYuvFormat(YuvFormat& yFormat, const std::string& format);
bool getYuvType(unsigned long long width, unsigned long long height, 
                unsigned long long fileSize, YuvFormat& yFormat);
bool isValidFormat(const std::string& format);
bool isValidNumber(const std::string& str);
uint8_t bilinearInterpolate(const std::vector<uint8_t>& src, unsigned int srcWidth, unsigned int srcHeight, double x, double y);

#endif // CONVERTERUTILS_H
