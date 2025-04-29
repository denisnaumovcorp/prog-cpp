#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_MEMORY_ALLOCATIONS 100

struct yuvFormat {
    unsigned short int format;
    uint8_t wMultiplier;
    uint8_t hMultiplier;
    uint8_t uvRatio;
};

extern const char *VALID_FORMATS[];

void initMemoryManager(void);
void* allocateMemory(size_t size);
uint8_t freeAllMemory(void);
uint8_t addToMemoryTracker(void* ptr);
uint8_t removeFromMemoryTracker(void* ptr);

void executeError(FILE *inputFile, FILE *outputFile, char *errorText);

unsigned long long getFileSize(FILE* f);
uint8_t setBmpHeader(FILE* f, unsigned int width, unsigned int height);

void initYuvFormat(struct yuvFormat *yFormat, const char *format);
uint8_t isValidFormat(const char *format);
uint8_t isValidNumber(const char* str);
uint8_t getYuvType(unsigned long long width, unsigned long long height, unsigned long long fileSize, struct yuvFormat *yFormat);

uint8_t bilinearInterpolate(uint8_t* src, unsigned int srcWidth, unsigned int srcHeight, double x, double y);
uint8_t changeYuvSize(uint8_t **y, uint8_t **cb, uint8_t **cr, unsigned long long *width, unsigned long long *height, unsigned short int downSizeK, unsigned short int upSizeK, struct yuvFormat *yFormat);
uint8_t changeBmpSize(uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long *width, unsigned long long *height, unsigned short int downSizeK, unsigned short int upSizeK);
void toGrayScaleYuv(uint8_t *cb, uint8_t *cr, unsigned long long *width, unsigned long long *height, struct yuvFormat *yFormat);
void toGrayScaleBmp(uint8_t *R, uint8_t *G, uint8_t *B, unsigned long long *width, unsigned long long *height);

uint8_t readBmpHeader(FILE *file, unsigned long long *width, unsigned long long *height, uint32_t *pixelOffset);
uint8_t readBmpPixelData(FILE *file, uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long width, unsigned long long height, uint32_t pixelOffset);
uint8_t writeBmpToFile(FILE *file, uint8_t *R, uint8_t *G, uint8_t *B, unsigned long long width, unsigned long long height);

uint8_t readYuvFile(FILE *file, uint8_t **Y, uint8_t **Cb, uint8_t **Cr, unsigned long long width, unsigned long long height, struct yuvFormat *format);
uint8_t writeYuvToFile(FILE *file, uint8_t *Y, uint8_t *Cb, uint8_t *Cr, unsigned long long width, unsigned long long height, struct yuvFormat *format);
uint8_t convertBmpToYuv(uint8_t *R, uint8_t *G, uint8_t *B, uint8_t **Y, uint8_t **Cb, uint8_t **Cr, unsigned long long width, unsigned long long height, struct yuvFormat *format);
uint8_t convertYuvToBmp(uint8_t *Y, uint8_t *Cb, uint8_t *Cr, uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long width, unsigned long long height, struct yuvFormat *format);
uint8_t convertYuvToYuv(uint8_t *Y, uint8_t *Cb, uint8_t *Cr, uint8_t **outY, uint8_t **outCb, uint8_t **outCr, unsigned long long width, unsigned long long height, struct yuvFormat *inputFormat, struct yuvFormat *outputFormat);

void compareImagesWithDifferentSizes(FILE *file1, FILE *file2, const char *format, 
                             unsigned long long width1, unsigned long long height1,
                             unsigned long long width2, unsigned long long height2);

void freeRgbBuffers(uint8_t *R, uint8_t *G, uint8_t *B);
void freeYuvBuffers(uint8_t *Y, uint8_t *Cb, uint8_t *Cr);