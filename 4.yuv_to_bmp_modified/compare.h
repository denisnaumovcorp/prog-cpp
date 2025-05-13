#ifndef COMPARE_H
#define COMPARE_H

#include "converterLibs.h"

void determineTargetSize(unsigned long long width1, unsigned long long height1,
                        unsigned long long width2, unsigned long long height2,
                        unsigned long long& targetWidth, unsigned long long& targetHeight);


void calculateScalingFactors(unsigned long long currentWidth, unsigned long long currentHeight,
                           unsigned long long targetWidth, unsigned long long targetHeight,
                           unsigned short int& upSize, unsigned short int& downSize);


void calculateRgbMetrics(const std::vector<uint8_t>& R1, const std::vector<uint8_t>& G1, const std::vector<uint8_t>& B1,
                        const std::vector<uint8_t>& R2, const std::vector<uint8_t>& G2, const std::vector<uint8_t>& B2,
                        unsigned long long width, unsigned long long height);

void calculateYuvMetrics(const std::vector<uint8_t>& Y1, const std::vector<uint8_t>& Cb1, const std::vector<uint8_t>& Cr1,
                        const std::vector<uint8_t>& Y2, const std::vector<uint8_t>& Cb2, const std::vector<uint8_t>& Cr2,
                        unsigned long long width, unsigned long long height, const YuvFormat& yuvFmt);

void compareImagesWithDifferentSizes(std::ifstream& file1, std::ifstream& file2, 
                                    const std::string& format,
                                    unsigned long long width1, unsigned long long height1,
                                    unsigned long long width2, unsigned long long height2);

#endif // COMPARE_H