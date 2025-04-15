#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct yuvFormat {
    unsigned short int format;
    uint8_t wMultiplier;
    uint8_t hMultiplier;
    uint8_t uvRatio;
};

void executeError(FILE *inputFile, FILE *outputFile, char *errorText);
unsigned long long getFileSize(FILE* f);
void getYuvType(unsigned int area, unsigned long long fileSize, struct yuvFormat *yFormat);
uint8_t setBmpHeader(FILE* f, unsigned int width, unsigned int height);
uint8_t bilinearInterpolate(uint8_t* src, unsigned int srcWidth, unsigned int srcHeight, double x, double y);
uint8_t isValidNumber(const char* str);
uint8_t changeSize(uint8_t **y, uint8_t **cb, uint8_t **cr, unsigned int *width, unsigned int *height, unsigned short int downSizeK, unsigned short int upSizeK, struct yuvFormat *yFormat);
void toGrayScale(uint8_t *cb, uint8_t *cr, unsigned int *width, unsigned int *height, struct yuvFormat *yFormat);