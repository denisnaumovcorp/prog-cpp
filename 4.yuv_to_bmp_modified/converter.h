#ifndef CONVERTER_H
#define CONVERTER_H

#include "converterLibs.h"   
#include "converterStructures.h"
#include "converterUtils.h"
#include "filesIO.h"
#include "compare.h"

bool convertBmpToYuv(const std::vector<uint8_t>& R, const std::vector<uint8_t>& G, 
                    const std::vector<uint8_t>& B, std::vector<uint8_t>& Y, 
                    std::vector<uint8_t>& Cb, std::vector<uint8_t>& Cr, 
                    unsigned long long width, unsigned long long height, 
                    const YuvFormat& format);

bool convertYuvToBmp(const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, 
                    const std::vector<uint8_t>& Cr, std::vector<uint8_t>& R, 
                    std::vector<uint8_t>& G, std::vector<uint8_t>& B, 
                    unsigned long long width, unsigned long long height, 
                    const YuvFormat& format);

bool convertYuvToYuv(const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, 
                     const std::vector<uint8_t>& Cr, std::vector<uint8_t>& outY, 
                     std::vector<uint8_t>& outCb, std::vector<uint8_t>& outCr, 
                     unsigned long long width, unsigned long long height, 
                     const YuvFormat& inputFormat, const YuvFormat& outputFormat);

bool changeSize(std::vector<uint8_t>& y, std::vector<uint8_t>& cb, 
                std::vector<uint8_t>& cr, unsigned long long& width, 
                unsigned long long& height, unsigned short int downSizeK, 
                unsigned short int upSizeK, const YuvFormat& yFormat);  

bool changeSize(std::vector<uint8_t>& R, std::vector<uint8_t>& G, 
                std::vector<uint8_t>& B, unsigned long long& width, 
                unsigned long long& height, unsigned short int downSizeK, 
                unsigned short int upSizeK);

void toGrayScale(std::vector<uint8_t>& cb, std::vector<uint8_t>& cr, 
                unsigned long long width, unsigned long long height, 
                const YuvFormat& yFormat);

void toGrayScale(std::vector<uint8_t>& R, std::vector<uint8_t>& G, 
                std::vector<uint8_t>& B, unsigned long long width, 
                unsigned long long height);

#endif