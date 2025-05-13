#include "converter.h"

void determineTargetSize(unsigned long long width1, unsigned long long height1,
                        unsigned long long width2, unsigned long long height2,
                        unsigned long long& targetWidth, unsigned long long& targetHeight) {
    if (width1 * height1 < width2 * height2) {
        targetWidth = width1;
        targetHeight = height1;
    } else {
        targetWidth = width2;
        targetHeight = height2;
    }
}

void calculateScalingFactors(unsigned long long currentWidth, unsigned long long currentHeight,
                           unsigned long long targetWidth, unsigned long long targetHeight,
                           unsigned short int& upSize, unsigned short int& downSize) {
    upSize = 1;
    downSize = 1;
    
    if (targetWidth > currentWidth) {
        upSize = targetWidth / currentWidth;
        upSize = (upSize == 0) ? 1 : upSize;
    } else {
        downSize = currentWidth / targetWidth;
        downSize = (downSize == 0) ? 1 : downSize;
    }
}

void calculateRgbMetrics(const std::vector<uint8_t>& R1, const std::vector<uint8_t>& G1, const std::vector<uint8_t>& B1,
                       const std::vector<uint8_t>& R2, const std::vector<uint8_t>& G2, const std::vector<uint8_t>& B2,
                       unsigned long long width, unsigned long long height) {
    double sumSqDiffR = 0, sumSqDiffG = 0, sumSqDiffB = 0;
    unsigned long long totalPixels = width * height;
    
    for (unsigned long long i = 0; i < height; i++) {
        for (unsigned long long j = 0; j < width; j++) {
            unsigned long long index = i * width + j;
            
            double diffR = R1[index] - R2[index];
            double diffG = G1[index] - G2[index];
            double diffB = B1[index] - B2[index];
            
            sumSqDiffR += diffR * diffR;
            sumSqDiffG += diffG * diffG;
            sumSqDiffB += diffB * diffB;
        }
    }
    
    double mseR = sumSqDiffR / totalPixels;
    double mseG = sumSqDiffG / totalPixels;
    double mseB = sumSqDiffB / totalPixels;
    double mseTotalRGB = (mseR + mseG + mseB) / 3.0;
    
    double psnr = 10.0 * log10((255.0 * 255.0) / (mseTotalRGB + 0.0001));
    double similarityPercent = 100.0 * (1.0 - sqrt(mseTotalRGB) / 255.0);
    
    std::cout << "Comparison Results:" << std::endl;
    std::cout << "MSE R channel: " << std::fixed << std::setprecision(2) << mseR << std::endl;
    std::cout << "MSE G channel: " << std::fixed << std::setprecision(2) << mseG << std::endl;
    std::cout << "MSE B channel: " << std::fixed << std::setprecision(2) << mseB << std::endl;
    std::cout << "Overall MSE: " << std::fixed << std::setprecision(2) << mseTotalRGB << std::endl;
    std::cout << "PSNR: " << std::fixed << std::setprecision(2) << psnr << " dB" << std::endl;
    std::cout << "Similarity: " << std::fixed << std::setprecision(2) << similarityPercent << "%" << std::endl;
}

void calculateYuvMetrics(const std::vector<uint8_t>& Y1, const std::vector<uint8_t>& Cb1, const std::vector<uint8_t>& Cr1,
                      const std::vector<uint8_t>& Y2, const std::vector<uint8_t>& Cb2, const std::vector<uint8_t>& Cr2,
                      unsigned long long width, unsigned long long height, const YuvFormat& yuvFmt) {
    double sumSqDiffY = 0, sumSqDiffCb = 0, sumSqDiffCr = 0;
    unsigned long long totalPixels = width * height;
    unsigned long long uvW = (width + yuvFmt.wMultiplier - 1) / yuvFmt.wMultiplier;
    unsigned long long uvH = (height + yuvFmt.hMultiplier - 1) / yuvFmt.hMultiplier;
    unsigned long long totalUvPixels = uvW * uvH;
    
    for (unsigned long long i = 0; i < totalPixels; i++) {
        double diffY = Y1[i] - Y2[i];
        sumSqDiffY += diffY * diffY;
    }
    
    for (unsigned long long i = 0; i < totalUvPixels; i++) {
        double diffCb = Cb1[i] - Cb2[i];
        double diffCr = Cr1[i] - Cr2[i];
        sumSqDiffCb += diffCb * diffCb;
        sumSqDiffCr += diffCr * diffCr;
    }
    
    double mseY = sumSqDiffY / totalPixels;
    double mseCb = sumSqDiffCb / totalUvPixels;
    double mseCr = sumSqDiffCr / totalUvPixels;
    
    double mseTotalYUV = (6.0 * mseY + mseCb + mseCr) / 8.0;
    
    double psnr = 10.0 * log10((255.0 * 255.0) / (mseTotalYUV + 0.0001));
    double similarityPercent = 100.0 * (1.0 - sqrt(mseTotalYUV) / 255.0);
    
    std::cout << "Comparison Results:" << std::endl;
    std::cout << "MSE Y channel: " << std::fixed << std::setprecision(2) << mseY << std::endl;
    std::cout << "MSE Cb channel: " << std::fixed << std::setprecision(2) << mseCb << std::endl;
    std::cout << "MSE Cr channel: " << std::fixed << std::setprecision(2) << mseCr << std::endl;
    std::cout << "Overall MSE: " << std::fixed << std::setprecision(2) << mseTotalYUV << std::endl;
    std::cout << "PSNR: " << std::fixed << std::setprecision(2) << psnr << " dB" << std::endl;
    std::cout << "Similarity: " << std::fixed << std::setprecision(2) << similarityPercent << "%" << std::endl;
}

void compareImagesWithDifferentSizes(std::ifstream& file1, std::ifstream& file2, 
                                    const std::string& format,
                                    unsigned long long width1, unsigned long long height1,
                                    unsigned long long width2, unsigned long long height2) {
    uint32_t pixelOffset1 = 0, pixelOffset2 = 0;
    bool isBmpFormat = (format == "bmp");
    if (isBmpFormat) {
        if (!readBmpHeader(file1, width1, height1, pixelOffset1)) {
            executeError(&file1, &file2, "Failed to read first BMP header");
        }
        if (!readBmpHeader(file2, width2, height2, pixelOffset2)) {
            executeError(&file1, &file2, "Failed to read second BMP header");
        }
    }
    std::cout << "Image 1: " << width1 << " x " << height1 
              << " (Aspect ratio: " << std::fixed << std::setprecision(3) << static_cast<double>(width1)/height1 << ")" << std::endl;
    std::cout << "Image 2: " << width2 << " x " << height2 
              << " (Aspect ratio: " << std::fixed << std::setprecision(3) << static_cast<double>(width2)/height2 << ")" << std::endl;

    unsigned long long targetWidth, targetHeight;
    determineTargetSize(width1, height1, width2, height2, targetWidth, targetHeight);
    
    if (isBmpFormat) {
        std::vector<uint8_t> R1, G1, B1;
        std::vector<uint8_t> R2, G2, B2;
        if (!readBmpPixelData(file1, R1, G1, B1, width1, height1, pixelOffset1)) {
            executeError(&file1, &file2, "Failed to read first BMP data");
        }
        if (!readBmpPixelData(file2, R2, G2, B2, width2, height2, pixelOffset2)) {
            executeError(&file1, &file2, "Failed to read second BMP data");
        }
        if (width1 != targetWidth || height1 != targetHeight) {
            unsigned short int upSize, downSize;
            calculateScalingFactors(width1, height1, targetWidth, targetHeight, upSize, downSize);
            
            if (!changeSize(R1, G1, B1, width1, height1, downSize, upSize)) {
                executeError(&file1, &file2, "Failed to resize first image");
            }
        }
        
        if (width2 != targetWidth || height2 != targetHeight) {
            unsigned short int upSize, downSize;
            calculateScalingFactors(width2, height2, targetWidth, targetHeight, upSize, downSize);
            
            if (!changeSize(R2, G2, B2, width2, height2, downSize, upSize)) {
                executeError(&file1, &file2, "Failed to resize second image");
            }
        }
        calculateRgbMetrics(R1, G1, B1, R2, G2, B2, targetWidth, targetHeight);
        
    } else {
        std::vector<uint8_t> Y1, Cb1, Cr1;
        std::vector<uint8_t> Y2, Cb2, Cr2;
        YuvFormat yuvFmt;
        
        initYuvFormat(yuvFmt, format);
        if (!readYuvFile(file1, Y1, Cb1, Cr1, width1, height1, yuvFmt)) {
            executeError(&file1, &file2, "Failed to read first YUV data");
        }
        if (!readYuvFile(file2, Y2, Cb2, Cr2, width2, height2, yuvFmt)) {
            executeError(&file1, &file2, "Failed to read second YUV data");   
        }
        if (width1 != targetWidth || height1 != targetHeight) {
            unsigned short int upSize, downSize;
            calculateScalingFactors(width1, height1, targetWidth, targetHeight, upSize, downSize);
            
            if (!changeSize(Y1, Cb1, Cr1, width1, height1, downSize, upSize, yuvFmt)) {
                executeError(&file1, &file2, "Failed to resize first YUV image");
            }
        }
        
        if (width2 != targetWidth || height2 != targetHeight) {
            unsigned short int upSize, downSize;
            calculateScalingFactors(width2, height2, targetWidth, targetHeight, upSize, downSize);
            
            if (!changeSize(Y2, Cb2, Cr2, width2, height2, downSize, upSize, yuvFmt)) {
                executeError(&file1, &file2, "Failed to resize second YUV image");
            }
        }
        calculateYuvMetrics(Y1, Cb1, Cr1, Y2, Cb2, Cr2, targetWidth, targetHeight, yuvFmt);
    }
}