#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

struct yuvFormat {
    unsigned short int format;
    uint8_t wMultiplier;
    uint8_t hMultiplier;
    uint8_t uvRatio;
};

void executeError(FILE *inputFile, FILE *outputFile, char *errorText) {
    if (!(inputFile)) {
        fclose(inputFile);
    }
    if (!(outputFile)) {
        fclose(inputFile);
    }
    fprintf(stderr, "%s", errorText);
    exit(EXIT_FAILURE);
}

unsigned long long getFileSize(FILE* f) {
    unsigned long long size = 0;
    fseek(f, 0, SEEK_END);
    size = ftello(f);
    rewind(f);
    return size;
}

void getYuvType(unsigned int area, unsigned long long fileSize, struct yuvFormat *yFormat) {
    printf("%d %d\n", area, fileSize);
    if (area * 3 == fileSize) {
        yFormat->hMultiplier = 1;
        yFormat->wMultiplier = 1;
        yFormat->uvRatio = 1;
        yFormat->format = 444;
    } else if (area * 2 == fileSize) {
        yFormat->hMultiplier = 1;
        yFormat->wMultiplier = 2;
        yFormat->uvRatio = 2;
        yFormat->format = 422;
    } else {
        yFormat->wMultiplier = 2;
        yFormat->hMultiplier = 2;
        yFormat->uvRatio = 4;
        yFormat->format = 420;
    }
}

uint8_t setBmpHeader(FILE* f, unsigned int width, unsigned int  height) {
    #pragma pack(push, 2)  
    struct bmpHeader {
        uint16_t type;         
        uint32_t size;         
        uint16_t reserved1;    
        uint16_t reserved2;    
        uint32_t offset;       
        uint32_t dib_size;     
        uint32_t width;        
        uint32_t height;       
        uint16_t planes;       
        uint16_t bpp;          
        uint32_t compression;  
        uint32_t image_size;   
        uint32_t x_ppm;        
        uint32_t y_ppm;           
        uint32_t num_colors;       
        uint32_t important_colors;  
    };
    #pragma pack(pop)
    struct bmpHeader header = {
        .type = 0x4D42,
        .size = 54 + (width * 3 + (4 - (width * 3) % 4) % 4) * height,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = 54,
        .dib_size = 40,
        .width = width,
        .height = height,
        .planes = 1,
        .bpp = 24,
        .compression = 0,
        .image_size = 0,
        .x_ppm = 0,
        .y_ppm = 0,   
        .num_colors = 0,
        .important_colors = 0
    };
    if (fwrite(&header, 54, 1, f) == 0) {
        return 0;
    }
    return 1;
}

uint8_t bilinearInterpolate(uint8_t* src, unsigned int srcWidth, unsigned int srcHeight, double x, double y) {
    int x1 = (int)x;
    int y1 = (int)y;
    int x2 = (x1 + 1 < srcWidth) ? x1 + 1 : x1;
    int y2 = (y1 + 1 < srcHeight) ? y1 + 1 : y1;
    double dx = x - x1;
    double dy = y - y1;
    unsigned char q11 = src[y1 * srcWidth + x1];
    unsigned char q12 = src[y2 * srcWidth + x1];
    unsigned char q21 = src[y1 * srcWidth + x2];
    unsigned char q22 = src[y2 * srcWidth + x2];
    unsigned char result = q11 * (1 - dx) * (1 - dy) +
                   q21 * dx * (1 - dy) +
                   q12 * (1 - dx) * dy +
                   q22 * dx * dy;
                   
    return result;
}

uint8_t changeSize(uint8_t **y, uint8_t **cb, uint8_t **cr, unsigned int *width, unsigned int *height, unsigned short int downSizeK, unsigned short int upSizeK, struct yuvFormat *yFormat) {
    int i, j;
    unsigned long long newHeight = *width * upSizeK / downSizeK;
    unsigned long long newWidth = *height * upSizeK / downSizeK;
    
    unsigned long long ySize = newHeight * newWidth;
    unsigned long long cbSize = ySize / yFormat->uvRatio;
    unsigned long long crSize = cbSize;
    
    uint8_t *nY = (uint8_t *)malloc(ySize);
    if (nY == NULL) {
        return 0;
    }
    uint8_t *nCb = (uint8_t *)malloc(cbSize);
    if (nCb == NULL) {
        free(nY);
        return 0; 
    }
    uint8_t *nCr = (uint8_t *)malloc(crSize);
    if (nCr == NULL) {
        free(nY);
        free(nCb);
        return 0;
    }
    
    for (i = newHeight - 1; i >= 0; i--) {
        for (j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            unsigned int chromaIndex = (i / yFormat->hMultiplier) * (newWidth / yFormat->wMultiplier) + (j / yFormat->wMultiplier);
            nY[i * newWidth + j] = bilinearInterpolate(*y, *width, *height, srcX, srcY);
            nCb[chromaIndex] = bilinearInterpolate(*cb, *width / yFormat->wMultiplier, *height / yFormat->hMultiplier, srcX / yFormat->wMultiplier, srcY / yFormat->hMultiplier);
            nCr[chromaIndex] = bilinearInterpolate(*cr, *width / yFormat->wMultiplier, *height / yFormat->hMultiplier, srcX / yFormat->wMultiplier, srcY / yFormat->hMultiplier);
        }
    }
    
    free(*y);
    free(*cb);
    free(*cr);
    *y = nY;
    *cb = nCb;
    *cr = nCr;
    *width = newWidth;
    *height = newHeight;
    return 1;
}

void toGrayScale(uint8_t *cb, uint8_t *cr, unsigned int *width, unsigned int *height, struct yuvFormat *yFormat) {
    int i, j;
    for (i = *height - 1; i >= 0; i--) {
        for (j = 0; j < *width; j++) {
            unsigned int chromaIndex = (i / yFormat->hMultiplier) * (*width / yFormat->wMultiplier) + (j / yFormat->wMultiplier);
            cb[chromaIndex] = 128;
            cr[chromaIndex] = 128;
        }
    }
}

uint8_t isValidNumber(const char* str) {
    if (str == NULL || *str == '\0') {
        return 0;
    }
    while (*str) {
        if (!isdigit(*str)) {
            return 0;
        }
        str++;
    }
    return 1;
}