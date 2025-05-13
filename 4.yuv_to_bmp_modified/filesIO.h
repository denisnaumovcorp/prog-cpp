#ifndef FILESIO_H
#define FILESIO_H

#include "converterStructures.h"

bool setBmpHeader(std::ofstream& file, unsigned int width, unsigned int height);
bool readBmpHeader(std::ifstream& file, unsigned long long& width, unsigned long long& height, uint32_t& pixelOffset);
bool readBmpPixelData(std::ifstream& file, std::vector<uint8_t>& R, std::vector<uint8_t>& G, std::vector<uint8_t>& B, unsigned long long width, unsigned long long height, uint32_t pixelOffset);
bool writeBmpToFile(std::ofstream& file, const std::vector<uint8_t>& R, const std::vector<uint8_t>& G, const std::vector<uint8_t>& B, unsigned long long width, unsigned long long height); 
bool readYuvFile(std::ifstream& file, std::vector<uint8_t>& Y, std::vector<uint8_t>& Cb, std::vector<uint8_t>& Cr, unsigned long long width, unsigned long long height, const YuvFormat& format);
bool writeYuvToFile(std::ofstream& file, const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, const std::vector<uint8_t>& Cr, unsigned long long width, unsigned long long height, const YuvFormat& format);

#endif // FILESIO_H
