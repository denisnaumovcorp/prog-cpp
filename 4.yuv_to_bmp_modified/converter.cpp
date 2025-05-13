#include "converter.h"

bool convertBmpToYuv(const std::vector<uint8_t>& R, const std::vector<uint8_t>& G, const std::vector<uint8_t>& B, 
                    std::vector<uint8_t>& Y, std::vector<uint8_t>& Cb, std::vector<uint8_t>& Cr, 
                    unsigned long long width, unsigned long long height, const YuvFormat& format) {
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format.wMultiplier - 1) / format.wMultiplier;
    unsigned long long uvH = (height + format.hMultiplier - 1) / format.hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    
    Y.resize(ySize);
    Cb.resize(chromaSize);
    Cr.resize(chromaSize);
    
    for (int i = height - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < width; j++) {
            unsigned long long chromaIndex = (i / format.hMultiplier) * uvW + (j / format.wMultiplier);
            double yVal = R[i * width + j] * 0.299 + G[i * width + j] * 0.587 + B[i * width + j] * 0.114;
            double cbVal = 128 + (-0.169 * R[i * width + j] - 0.331 * G[i * width + j] + 0.5 * B[i * width + j]);
            double crVal = 128 + (0.5 * R[i * width + j] - 0.419 * G[i * width + j] - 0.081 * B[i * width + j]);
            Y[i * width + j] = static_cast<uint8_t>(std::clamp(yVal, 0.0, 255.0));
            Cb[chromaIndex] = static_cast<uint8_t>(std::clamp(cbVal, 0.0, 255.0));
            Cr[chromaIndex] = static_cast<uint8_t>(std::clamp(crVal, 0.0, 255.0));
        }
    }
    return true;
}

bool convertYuvToBmp(const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, const std::vector<uint8_t>& Cr, 
                    std::vector<uint8_t>& R, std::vector<uint8_t>& G, std::vector<uint8_t>& B, 
                    unsigned long long width, unsigned long long height, const YuvFormat& format) {
    unsigned long long size = width * height;
    unsigned long long uvW = (width + format.wMultiplier - 1) / format.wMultiplier;
    R.resize(size);
    G.resize(size);
    B.resize(size);
    for (int i = height - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < width; j++) {
            unsigned long long chromaIndex = (i / format.hMultiplier) * uvW + (j / format.wMultiplier);
            double rVal = Y[i * width + j] + 1.4 * (Cr[chromaIndex] - 128);
            double gVal = Y[i * width + j] - 0.343 * (Cb[chromaIndex] - 128) - 0.711 * (Cr[chromaIndex] - 128);
            double bVal = Y[i * width + j] + 1.762 * (Cb[chromaIndex] - 128);
            R[i * width + j] = static_cast<uint8_t>(std::clamp(rVal, 0.0, 255.0));
            G[i * width + j] = static_cast<uint8_t>(std::clamp(gVal, 0.0, 255.0));
            B[i * width + j] = static_cast<uint8_t>(std::clamp(bVal, 0.0, 255.0));
        }
    }
    return true;
}

bool convertYuvToYuv(const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, const std::vector<uint8_t>& Cr, 
                     std::vector<uint8_t>& outY, std::vector<uint8_t>& outCb, std::vector<uint8_t>& outCr, 
                     unsigned long long width, unsigned long long height, 
                     const YuvFormat& inputFormat, const YuvFormat& outputFormat) {
    unsigned long long ySize = width * height;
    unsigned long long inputUvW = (width + inputFormat.wMultiplier - 1) / inputFormat.wMultiplier;
    unsigned long long inputUvH = (height + inputFormat.hMultiplier - 1) / inputFormat.hMultiplier;
    unsigned long long outputUvW = (width + outputFormat.wMultiplier - 1) / outputFormat.wMultiplier;
    unsigned long long outputUvH = (height + outputFormat.hMultiplier - 1) / outputFormat.hMultiplier;
    unsigned long long outputChromaSize = outputUvW * outputUvH;
    outY.resize(ySize);
    outCb.resize(outputChromaSize);
    outCr.resize(outputChromaSize);
    std::copy(Y.begin(), Y.end(), outY.begin());
    for (int i = 0; i < height; i++) {
        for (unsigned long long j = 0; j < width; j++) {
            unsigned long long inputChromaIndex = (i / inputFormat.hMultiplier) * inputUvW + (j / inputFormat.wMultiplier);
            unsigned long long outputChromaIndex = (i / outputFormat.hMultiplier) * outputUvW + (j / outputFormat.wMultiplier);
            
            if ((i % outputFormat.hMultiplier == 0) && (j % outputFormat.wMultiplier == 0)) {
                if (inputFormat.uvRatio > outputFormat.uvRatio) {
                    outCb[outputChromaIndex] = Cb[inputChromaIndex];
                    outCr[outputChromaIndex] = Cr[inputChromaIndex];
                } 
                else if (inputFormat.uvRatio < outputFormat.uvRatio) {
                    int sum_cb = 0, sum_cr = 0, count = 0;
                    for (int ii = 0; ii < outputFormat.hMultiplier; ii += inputFormat.hMultiplier) {
                        for (int jj = 0; jj < outputFormat.wMultiplier; jj += inputFormat.wMultiplier) {
                            if (i + ii < height && j + jj < width) {
                                unsigned long long idx = ((i + ii) / inputFormat.hMultiplier) * inputUvW + 
                                                 ((j + jj) / inputFormat.wMultiplier);
                                sum_cb += Cb[idx];
                                sum_cr += Cr[idx];
                                count++;
                            }
                        }
                    }
                    outCb[outputChromaIndex] = (count > 0) ? (sum_cb / count) : 128;
                    outCr[outputChromaIndex] = (count > 0) ? (sum_cr / count) : 128;
                }
                else {
                    outCb[outputChromaIndex] = Cb[inputChromaIndex];
                    outCr[outputChromaIndex] = Cr[inputChromaIndex];
                }
            }
        }
    }
    return true;
}

bool changeSize(std::vector<uint8_t>& y, std::vector<uint8_t>& cb, std::vector<uint8_t>& cr, 
                  unsigned long long& width, unsigned long long& height, 
                  unsigned short int downSizeK, unsigned short int upSizeK, 
                  const YuvFormat& yFormat) {
    unsigned long long newWidth = width * upSizeK / downSizeK;
    unsigned long long newHeight = height * upSizeK / downSizeK;
    unsigned long long ySize = newWidth * newHeight;
    unsigned long long uvW = (newWidth + yFormat.wMultiplier - 1) / yFormat.wMultiplier;
    unsigned long long uvH = (newHeight + yFormat.hMultiplier - 1) / yFormat.hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    
    std::vector<uint8_t> nY(ySize);
    std::vector<uint8_t> nCb(chromaSize);
    std::vector<uint8_t> nCr(chromaSize);
    
    for (int i = newHeight - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            unsigned int chromaIndex = (i / yFormat.hMultiplier) * uvW + (j / yFormat.wMultiplier);
            
            nY[i * newWidth + j] = bilinearInterpolate(y, width, height, srcX, srcY);
            nCb[chromaIndex] = bilinearInterpolate(cb, width / yFormat.wMultiplier, height / yFormat.hMultiplier, 
                                                 srcX / yFormat.wMultiplier, srcY / yFormat.hMultiplier);
            nCr[chromaIndex] = bilinearInterpolate(cr, width / yFormat.wMultiplier, height / yFormat.hMultiplier, 
                                                 srcX / yFormat.wMultiplier, srcY / yFormat.hMultiplier);
        }
    }
    
    y = std::move(nY);
    cb = std::move(nCb);
    cr = std::move(nCr);
    width = newWidth;
    height = newHeight;
    
    return true;
}

bool changeSize(std::vector<uint8_t>& R, std::vector<uint8_t>& G, std::vector<uint8_t>& B, 
                  unsigned long long& width, unsigned long long& height, 
                  unsigned short int downSizeK, unsigned short int upSizeK) {
    unsigned long long newWidth = width * upSizeK / downSizeK;
    unsigned long long newHeight = height * upSizeK / downSizeK;
    unsigned long long newSize = newWidth * newHeight;
    
    std::vector<uint8_t> nR(newSize);
    std::vector<uint8_t> nG(newSize);
    std::vector<uint8_t> nB(newSize);
    
    for (long long i = newHeight - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            
            nR[i * newWidth + j] = bilinearInterpolate(R, width, height, srcX, srcY);
            nG[i * newWidth + j] = bilinearInterpolate(G, width, height, srcX, srcY);
            nB[i * newWidth + j] = bilinearInterpolate(B, width, height, srcX, srcY);
        }
    }
    
    R = std::move(nR);
    G = std::move(nG);
    B = std::move(nB);
    width = newWidth;
    height = newHeight;
    
    return true;
}

void toGrayScale(std::vector<uint8_t>& cb, std::vector<uint8_t>& cr, 
                    unsigned long long width, unsigned long long height, 
                    const YuvFormat& yFormat) {
    unsigned long long uvW = width / yFormat.wMultiplier;
    unsigned long long uvH = height / yFormat.hMultiplier;
    
    std::fill(cb.begin(), cb.end(), 128);
    std::fill(cr.begin(), cr.end(), 128);
}

void toGrayScale(std::vector<uint8_t>& R, std::vector<uint8_t>& G, std::vector<uint8_t>& B, 
                    unsigned long long width, unsigned long long height) {
    for (int i = height - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < width; j++) {
            unsigned long long index = i * width + j;
            uint8_t grayValue = (R[index] + G[index] + B[index]) / 3;
            R[index] = grayValue;
            G[index] = grayValue;
            B[index] = grayValue;
        }
    }
}

