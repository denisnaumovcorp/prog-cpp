#include <stdio.h>

unsigned long long getFileSize(FILE* f);
unsigned int getYuvType(unsigned int area, unsigned long long fileSize);
void setBmpHeader(FILE* f, unsigned int width, unsigned int height);
unsigned char bilinearInterpolate(unsigned char* src, int srcWidth, int srcHeight, double x, double y);