#include <stdlib.h>
#include <stdio.h>
#include "converter.h"

int main(int argc, char *argv[]) {
    unsigned int width = 0, height = 0;
    char notToGrayScale = 1, upSizeK = 1, downSizeK = 1;
    long long i, j;
    FILE *inputFile = NULL, *outputFile = NULL;
    for (i = 0; i < argc; i++) {
        switch(argv[i][2]) {
        case 'i':
            inputFile = fopen(argv[++i], "rb");
            break;
        case 'o':
            outputFile = fopen(argv[++i], "wb");
            break;
        case 'w':
            width = strtoll(argv[++i], NULL, 10);
            break;
        case 'h':
            height = strtoll(argv[++i], NULL, 10);
            break;
        case 'u':
            upSizeK = strtoll(argv[++i], NULL, 10);
            break;
        case 'd':
            downSizeK = strtoll(argv[++i], NULL, 10);
            break;
        case 't':
            notToGrayScale = 0;
            break;
        default:
            break;
        }
    }
    unsigned int newWidth = width * upSizeK / downSizeK;
    unsigned int newHeight = height * upSizeK / downSizeK;
    setBmpHeader(outputFile, newWidth, newHeight);
    unsigned long long yuvFileSize = getFileSize(inputFile), ySize = width * height, cbSize, crSize;
    unsigned short int yuvFormat = getYuvType(width * height, yuvFileSize), wMultiplier, hMultiplier, bmpPadding = (4 - (newWidth * 3) % 4) % 4;;
    switch (yuvFormat) {
    case 444:
        hMultiplier = 1;
        wMultiplier = 1;
        cbSize = ySize;
        crSize = ySize;
        break;
    case 422:
        hMultiplier = 1;
        wMultiplier = 2;
        cbSize = ySize / 2;
        crSize = ySize / 2;
        break;
    case 420:
        hMultiplier = 2;
        wMultiplier = 2;
        cbSize = ySize / 4;
        crSize = ySize / 4;
        break;
    default:
        break;
    }
    unsigned char *y = (unsigned char *)malloc(ySize);
    unsigned char *cb = (unsigned char *)malloc(cbSize);
    unsigned char *cr = (unsigned char *)malloc(crSize);
    fread(y, 1, ySize, inputFile);
    fread(cb, 1, cbSize, inputFile);
    fread(cr, 1, crSize, inputFile);
    for (i = newHeight - 1; i >= 0; i--) {
        for (j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            unsigned char yVal = bilinearInterpolate(y, width, height, srcX, srcY);
            double cbcrX = srcX / wMultiplier;
            double cbcrY = srcY / hMultiplier;
            unsigned char cbVal = bilinearInterpolate(cb, width/wMultiplier, height/hMultiplier, cbcrX, cbcrY);
            unsigned char crVal = bilinearInterpolate(cr, width/wMultiplier, height/hMultiplier, cbcrX, cbcrY);
            int rVal = yVal + 1.4 * (crVal - 128) * notToGrayScale;
            int gVal = yVal - 0.343 * (cbVal - 128) * notToGrayScale - 0.711 * (crVal - 128) * notToGrayScale;
            int bVal = yVal + 1.762 * (cbVal - 128) * notToGrayScale;
            unsigned char R = (rVal < 0) ? 0 : ((rVal > 255) ? 255 : rVal);
            unsigned char G = (gVal < 0) ? 0 : ((gVal > 255) ? 255 : gVal);
            unsigned char B = (bVal < 0) ? 0 : ((bVal > 255) ? 255 : bVal);
            putc(B, outputFile);
            putc(G, outputFile);
            putc(R, outputFile);
        }
        for (int p = 0; p < bmpPadding; p++) {
            putc(0, outputFile);
        }
    }
    free(y);
    free(cr);
    free(cb);
    fclose(inputFile);
    fclose(outputFile);
    return 0;
}
