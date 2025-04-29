#include "converter.h"

const char *VALID_FORMATS[] = {"yuv444", "yuv422", "yuv420", "bmp"};

static void* memoryAllocations[MAX_MEMORY_ALLOCATIONS] = {NULL};
static int allocationCount = 0;

void initMemoryManager(void) {
    for (int i = 0; i < MAX_MEMORY_ALLOCATIONS; i++) {
        memoryAllocations[i] = NULL;
    }
    allocationCount = 0;
}

void* allocateMemory(size_t size) {
    void* ptr = malloc(size);
    if (ptr != NULL) {
        if (!addToMemoryTracker(ptr)) {
            free(ptr);
            return NULL;
        }
    }
    return ptr;
}

uint8_t addToMemoryTracker(void* ptr) {
    if (allocationCount >= MAX_MEMORY_ALLOCATIONS) {
        fprintf(stderr, "Error: Maximum number of memory allocations reached\n");
        return 0;
    }
    memoryAllocations[allocationCount++] = ptr;
    return 1;
}

uint8_t removeFromMemoryTracker(void* ptr) {
    for (int i = 0; i < allocationCount; i++) {
        if (memoryAllocations[i] == ptr) {
            for (int j = i; j < allocationCount - 1; j++) {
                memoryAllocations[j] = memoryAllocations[j + 1];
            }
            memoryAllocations[allocationCount - 1] = NULL;
            allocationCount--;
            return 1;
        }
    }
    return 0;
}

uint8_t freeAllMemory(void) {
    for (int i = 0; i < allocationCount; i++) {
        if (memoryAllocations[i] != NULL) {
            free(memoryAllocations[i]);
            memoryAllocations[i] = NULL;
        }
    }
    allocationCount = 0;
    return 1;
}

void executeError(FILE *inputFile, FILE *outputFile, char *errorText) {
    if (inputFile != NULL) {
        fclose(inputFile);
    }
    if (outputFile != NULL) {
        fclose(outputFile);
    }
    freeAllMemory();
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

void initYuvFormat(struct yuvFormat *yFormat, const char *format) {
    if (strcmp(format, "yuv444") == 0) {
        yFormat->format = 444;
        yFormat->wMultiplier = 1;
        yFormat->hMultiplier = 1;
        yFormat->uvRatio = 1;
    } else if (strcmp(format, "yuv422") == 0) {
        yFormat->format = 422;
        yFormat->wMultiplier = 2;
        yFormat->hMultiplier = 1;
        yFormat->uvRatio = 2;
    } else if (strcmp(format, "yuv420") == 0) {
        yFormat->format = 420;
        yFormat->wMultiplier = 2;
        yFormat->hMultiplier = 2;
        yFormat->uvRatio = 4;
    }
}

uint8_t getYuvType(unsigned long long width, unsigned long long height, unsigned long long fileSize, struct yuvFormat *yFormat) {
    unsigned long long area = width * height;
    if (fileSize == area * 3) {
        yFormat->hMultiplier = 1;
        yFormat->wMultiplier = 1;
        yFormat->uvRatio = 1;
        yFormat->format = 444;
    } else if (fileSize == area + 2 * ((width + 1) / 2) * height) {
        yFormat->hMultiplier = 1;
        yFormat->wMultiplier = 2;
        yFormat->uvRatio = 2;
        yFormat->format = 422;
    } else if (fileSize == area + 2 * ((width + 1) / 2) * ((height + 1) / 2)) {
        yFormat->wMultiplier = 2;
        yFormat->hMultiplier = 2;
        yFormat->uvRatio = 4;
        yFormat->format = 420;
    } else {
        return 0;
    }
    return 1;
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

uint8_t isValidFormat(const char *format) {
    for (int i = 0; i < sizeof(VALID_FORMATS) / sizeof(VALID_FORMATS[0]); i++) {
        if (strcmpi(format, VALID_FORMATS[i]) == 0) {
            return 1;
        }
    }
    return 0;   
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

uint8_t readBmpHeader(FILE *file, unsigned long long *width, unsigned long long *height, uint32_t *pixelOffset) {
    uint32_t fileSize;
    
    fseek(file, 2, SEEK_SET);
    if (fread(&fileSize, 4, 1, file) != 1) {
        return 0;
    }
    
    fseek(file, 10, SEEK_SET);
    if (fread(pixelOffset, 4, 1, file) != 1) {
        return 0;
    }
    
    fseek(file, 18, SEEK_SET);
    if (fread(width, 4, 1, file) != 1) {
        return 0;
    }
    
    if (fread(height, 4, 1, file) != 1) {
        return 0;
    }
    
    return 1;
}

uint8_t readBmpPixelData(FILE *file, uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long width, unsigned long long height, uint32_t pixelOffset) {
    int i, j;
    unsigned long long rgbSize = width * height;
    int padding = (4 - ((width * 3) % 4)) % 4;
    
    fseek(file, pixelOffset, SEEK_SET);
    
    *R = (uint8_t *)allocateMemory(rgbSize);
    *G = (uint8_t *)allocateMemory(rgbSize);
    *B = (uint8_t *)allocateMemory(rgbSize);
    
    if (*R == NULL || *G == NULL || *B == NULL) {
        return 0;
    }
    
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            uint8_t b = fgetc(file);
            uint8_t g = fgetc(file);
            uint8_t r = fgetc(file);
            
            if (b == EOF || g == EOF || r == EOF) {
                return 0;
            }
            
            (*B)[i * width + j] = b;
            (*G)[i * width + j] = g;
            (*R)[i * width + j] = r;
        }
        
        for (int p = 0; p < padding; p++) {
            fgetc(file);
        }
    }
    
    return 1;
}

uint8_t writeBmpToFile(FILE *file, uint8_t *R, uint8_t *G, uint8_t *B, 
                       unsigned long long width, unsigned long long height) {
    int i, j;
    int padding = (4 - ((width * 3) % 4)) % 4;
    
    if (setBmpHeader(file, width, height) == 0) {
        return 0;
    }
    
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            if (fputc(B[i * width + j], file) == EOF ||
                fputc(G[i * width + j], file) == EOF ||
                fputc(R[i * width + j], file) == EOF) {
                return 0;
            }
        }
        
        if (fwrite("\0\0\0\0", 1, padding, file) != padding) {
            return 0;
        }
    }
    
    return 1;
}

uint8_t readYuvFile(FILE *file, uint8_t **Y, uint8_t **Cb, uint8_t **Cr, 
                   unsigned long long width, unsigned long long height, struct yuvFormat *format) {
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format->wMultiplier - 1) / format->wMultiplier;
    unsigned long long uvH = (height + format->hMultiplier - 1) / format->hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    *Y = (uint8_t *)allocateMemory(ySize);
    *Cb = (uint8_t *)allocateMemory(chromaSize);
    *Cr = (uint8_t *)allocateMemory(chromaSize);
    
    if (*Y == NULL || *Cb == NULL || *Cr == NULL) {
        return 0;
    }
    
    if (fread(*Y, 1, ySize, file) == 0 || 
        fread(*Cb, 1, chromaSize, file) == 0 || 
        fread(*Cr, 1, chromaSize, file) == 0) {
        return 0;
    }
    
    return 1;
}

uint8_t writeYuvToFile(FILE *file, uint8_t *Y, uint8_t *Cb, uint8_t *Cr, unsigned long long width, unsigned long long height, struct yuvFormat *format) {
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format->wMultiplier - 1) / format->wMultiplier;
    unsigned long long uvH = (height + format->hMultiplier - 1) / format->hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    
    if (fwrite(Y, 1, ySize, file) == 0 || 
        fwrite(Cb, 1, chromaSize, file) == 0 || 
        fwrite(Cr, 1, chromaSize, file) == 0) {
        return 0;
    }
    
    return 1;
}

uint8_t convertBmpToYuv(uint8_t *R, uint8_t *G, uint8_t *B, uint8_t **Y, uint8_t **Cb, uint8_t **Cr, unsigned long long width, unsigned long long height, struct yuvFormat *format) {
    int i, j;
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format->wMultiplier - 1) / format->wMultiplier;
    unsigned long long uvH = (height + format->hMultiplier - 1) / format->hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    *Y = (uint8_t *)allocateMemory(ySize);
    *Cb = (uint8_t *)allocateMemory(chromaSize);
    *Cr = (uint8_t *)allocateMemory(chromaSize);
    
    if (*Y == NULL || *Cb == NULL || *Cr == NULL) {
        return 0;
    }
    
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            unsigned long long chromaIndex = (i / format->hMultiplier) * uvW + (j / format->wMultiplier);
            double yVal = R[i * width + j] * 0.299 + G[i * width + j] * 0.587 + B[i * width + j] * 0.114;
            double cbVal = 128 + (-0.169 * R[i * width + j] - 0.331 * G[i * width + j] + 0.5 * B[i * width + j]);
            double crVal = 128 + (0.5 * R[i * width + j] - 0.419 * G[i * width + j] - 0.081 * B[i * width + j]);
            
            (*Y)[i * width + j] = (uint8_t)(yVal < 0 ? 0 : (yVal > 255 ? 255 : yVal));
            (*Cb)[chromaIndex] = (uint8_t)(cbVal < 0 ? 0 : (cbVal > 255 ? 255 : cbVal));
            (*Cr)[chromaIndex] = (uint8_t)(crVal < 0 ? 0 : (crVal > 255 ? 255 : crVal));
        }
    }
    return 1;
}

uint8_t convertYuvToBmp(uint8_t *Y, uint8_t *Cb, uint8_t *Cr, uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long width, unsigned long long height, struct yuvFormat *format) {
    int i, j;
    unsigned long long size = width * height;
    unsigned long long uvW = (width + format->wMultiplier - 1) / format->wMultiplier;
    *R = (uint8_t *)allocateMemory(size);
    *G = (uint8_t *)allocateMemory(size);
    *B = (uint8_t *)allocateMemory(size);
    
    if (*R == NULL || *G == NULL || *B == NULL) {
        return 0;
    }
    
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            unsigned long long chromaIndex = (i / format->hMultiplier) * uvW + (j / format->wMultiplier);
            short int rVal = Y[i * width + j] + 1.4 * (Cr[chromaIndex] - 128);
            short int gVal = Y[i * width + j] - 0.343 * (Cb[chromaIndex] - 128) - 0.711 * (Cr[chromaIndex] - 128);
            short int bVal = Y[i * width + j] + 1.762 * (Cb[chromaIndex] - 128);
            (*R)[i * width + j] = (rVal < 0) ? 0 : ((rVal > 255) ? 255 : rVal);
            (*G)[i * width + j] = (gVal < 0) ? 0 : ((gVal > 255) ? 255 : gVal);
            (*B)[i * width + j] = (bVal < 0) ? 0 : ((bVal > 255) ? 255 : bVal);
        }
    }
    return 1;
}

uint8_t convertYuvToYuv(uint8_t *Y, uint8_t *Cb, uint8_t *Cr, uint8_t **outY, uint8_t **outCb, uint8_t **outCr, unsigned long long width, unsigned long long height, struct yuvFormat *inputFormat, struct yuvFormat *outputFormat) {
    unsigned long long ySize = width * height;
    unsigned long long inputUvW = (width + inputFormat->wMultiplier - 1) / inputFormat->wMultiplier;
    unsigned long long inputUvH = (height + inputFormat->hMultiplier - 1) / inputFormat->hMultiplier;
    unsigned long long outputUvW = (width + outputFormat->wMultiplier - 1) / outputFormat->wMultiplier;
    unsigned long long outputUvH = (height + outputFormat->hMultiplier - 1) / outputFormat->hMultiplier;
    unsigned long long outputChromaSize = outputUvW * outputUvH;
    *outY = (uint8_t *)allocateMemory(ySize);
    *outCb = (uint8_t *)allocateMemory(outputChromaSize);
    *outCr = (uint8_t *)allocateMemory(outputChromaSize);

    if (*outY == NULL || *outCb == NULL || *outCr == NULL) {
        return 0;
    }
    memcpy(*outY, Y, ySize);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            unsigned long long inputChromaIndex = (i / inputFormat->hMultiplier) * inputUvW + (j / inputFormat->wMultiplier);
            unsigned long long outputChromaIndex = (i / outputFormat->hMultiplier) * outputUvW + (j / outputFormat->wMultiplier);
            if ((i % outputFormat->hMultiplier == 0) && (j % outputFormat->wMultiplier == 0)) {
                if (inputFormat->uvRatio > outputFormat->uvRatio) {
                    (*outCb)[outputChromaIndex] = Cb[inputChromaIndex];
                    (*outCr)[outputChromaIndex] = Cr[inputChromaIndex];
                } 
                else if (inputFormat->uvRatio < outputFormat->uvRatio) {
                    int sum_cb = 0, sum_cr = 0, count = 0;
                    for (int ii = 0; ii < outputFormat->hMultiplier; ii += inputFormat->hMultiplier) {
                        for (int jj = 0; jj < outputFormat->wMultiplier; jj += inputFormat->wMultiplier) {
                            if (i + ii < height && j + jj < width) {
                                unsigned long long idx = ((i + ii) / inputFormat->hMultiplier) * inputUvW + 
                                                 ((j + jj) / inputFormat->wMultiplier);
                                sum_cb += Cb[idx];
                                sum_cr += Cr[idx];
                                count++;
                            }
                        }
                    }
                    (*outCb)[outputChromaIndex] = (count > 0) ? (sum_cb / count) : 128;
                    (*outCr)[outputChromaIndex] = (count > 0) ? (sum_cr / count) : 128;
                }
                else {
                    (*outCb)[outputChromaIndex] = Cb[inputChromaIndex];
                    (*outCr)[outputChromaIndex] = Cr[inputChromaIndex];
                }
            }
        }
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
    uint8_t q11 = src[y1 * srcWidth + x1];
    uint8_t q12 = src[y2 * srcWidth + x1];
    uint8_t q21 = src[y1 * srcWidth + x2];
    uint8_t q22 = src[y2 * srcWidth + x2];
    uint8_t result = q11 * (1 - dx) * (1 - dy) +
                   q21 * dx * (1 - dy) +
                   q12 * (1 - dx) * dy +
                   q22 * dx * dy;
    return result;
}

uint8_t changeYuvSize(uint8_t **y, uint8_t **cb, uint8_t **cr, unsigned long long *width, unsigned long long *height, unsigned short int downSizeK, unsigned short int upSizeK, struct yuvFormat *yFormat) {
    int i, j;
    unsigned long long newWidth = *width * upSizeK / downSizeK;
    unsigned long long newHeight = *height * upSizeK / downSizeK;
    unsigned long long ySize = newWidth * newHeight;
    unsigned long long uvW = (newWidth + yFormat->wMultiplier - 1) / yFormat->wMultiplier;
    unsigned long long uvH = (newHeight + yFormat->hMultiplier - 1) / yFormat->hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    uint8_t *nY = (uint8_t *)allocateMemory(ySize);
    if (nY == NULL) {
        return 0;
    }
    uint8_t *nCb = (uint8_t *)allocateMemory(chromaSize);
    if (nCb == NULL) {
        return 0; 
    }
    uint8_t *nCr = (uint8_t *)allocateMemory(chromaSize);
    if (nCr == NULL) {
        return 0;
    }
    
    for (i = newHeight - 1; i >= 0; i--) {
        for (j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            unsigned int chromaIndex = (i / yFormat->hMultiplier) * uvW + (j / yFormat->wMultiplier);
            nY[i * newWidth + j] = bilinearInterpolate(*y, *width, *height, srcX, srcY);
            nCb[chromaIndex] = bilinearInterpolate(*cb, *width / yFormat->wMultiplier, *height / yFormat->hMultiplier, srcX / yFormat->wMultiplier, srcY / yFormat->hMultiplier);
            nCr[chromaIndex] = bilinearInterpolate(*cr, *width / yFormat->wMultiplier, *height / yFormat->hMultiplier, srcX / yFormat->wMultiplier, srcY / yFormat->hMultiplier);
        }
    }
    removeFromMemoryTracker(*y);
    removeFromMemoryTracker(*cb);
    removeFromMemoryTracker(*cr);
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

uint8_t changeBmpSize(uint8_t **R, uint8_t **G, uint8_t **B, unsigned long long *width, unsigned long long *height, unsigned short int downSizeK, unsigned short int upSizeK) {
    long long i, j;
    unsigned long long newWidth = *width * upSizeK / downSizeK;
    unsigned long long newHeight = *height * upSizeK / downSizeK;
    unsigned long long newSize = newWidth * newHeight;
    
    uint8_t *nR = (uint8_t *)allocateMemory(newSize);
    if (nR == NULL) {
        return 0;
    }
    
    uint8_t *nG = (uint8_t *)allocateMemory(newSize);
    if (nG == NULL) {
        return 0; 
    }
    
    uint8_t *nB = (uint8_t *)allocateMemory(newSize);
    if (nB == NULL) {
        return 0;
    }

    for (i = newHeight - 1; i >= 0; i--) {
        for (j = 0; j < newWidth; j++) {
            double srcX = j * downSizeK * 1.0 / upSizeK;
            double srcY = i * downSizeK * 1.0 / upSizeK;
            nR[i * newWidth + j] = bilinearInterpolate(*R, *width, *height, srcX, srcY);
            nG[i * newWidth + j] = bilinearInterpolate(*G, *width, *height, srcX, srcY);
            nB[i * newWidth + j] = bilinearInterpolate(*B, *width, *height, srcX, srcY);
        }
    }
    removeFromMemoryTracker(*R);
    removeFromMemoryTracker(*G);
    removeFromMemoryTracker(*B);
    free(*R);
    free(*G);
    free(*B);
    *R = nR;
    *G = nG;
    *B = nB;
    *width = newWidth;
    *height = newHeight;
    return 1;
}

void toGrayScaleYuv(uint8_t *cb, uint8_t *cr, unsigned long long *width, unsigned long long *height, struct yuvFormat *yFormat) {
    int i, j;
    for (i = *height - 1; i >= 0; i--) {
        for (j = 0; j < *width; j++) {
            unsigned int chromaIndex = (i / yFormat->hMultiplier) * (*width / yFormat->wMultiplier) + (j / yFormat->wMultiplier);
            cb[chromaIndex] = 128;
            cr[chromaIndex] = 128;
        }
    }
}

void toGrayScaleBmp(uint8_t *R, uint8_t *G, uint8_t *B, unsigned long long *width, unsigned long long *height) {
    int i, j;
    for (i = *height - 1; i >= 0; i--) {
        for (j = 0; j < *width; j++) {
            uint8_t grayValue = (R[i * *width + j] + G[i * *width + j] + B[i * *width + j]) / 3;
            R[i * *width + j] = grayValue;
            G[i * *width + j] = grayValue;
            B[i * *width + j] = grayValue;
        }
    }
}

void compareImagesWithDifferentSizes(FILE *file1, FILE *file2, const char *format, 
                             unsigned long long width1, unsigned long long height1,
                             unsigned long long width2, unsigned long long height2) {
    uint32_t pixelOffset1 = 0, pixelOffset2 = 0;
    uint8_t isBmpFormat = (strcmp(format, "bmp") == 0);
    
    if (isBmpFormat) {
        if (!readBmpHeader(file1, &width1, &height1, &pixelOffset1)) {
            executeError(file1, file2, "Failed to read first BMP header");
        }
        if (!readBmpHeader(file2, &width2, &height2, &pixelOffset2)) {
            executeError(file1, file2, "Failed to read second BMP header");
        }
    }
    
    printf("Image 1: %llu x %llu (Aspect ratio: %.3f)\n", width1, height1, (double)width1/height1);
    printf("Image 2: %llu x %llu (Aspect ratio: %.3f)\n", width2, height2, (double)width2/height2);
    
    double aspectRatio1 = (double)width1 / height1;
    double aspectRatio2 = (double)width2 / height2;
    
    uint8_t *R1 = NULL, *G1 = NULL, *B1 = NULL;
    uint8_t *R2 = NULL, *G2 = NULL, *B2 = NULL;
    uint8_t *Y1 = NULL, *Cb1 = NULL, *Cr1 = NULL;
    uint8_t *Y2 = NULL, *Cb2 = NULL, *Cr2 = NULL;
    
    if (isBmpFormat) {
        if (!readBmpPixelData(file1, &R1, &G1, &B1, width1, height1, pixelOffset1)) {
            executeError(file1, file2, "Failed to read first BMP data");
        }
        if (!readBmpPixelData(file2, &R2, &G2, &B2, width2, height2, pixelOffset2)) {
            executeError(file1, file2, "Failed to read second BMP data");
        }
        
        unsigned long long targetWidth, targetHeight;
        
        if (width1 * height1 < width2 * height2) {
            targetWidth = width1;
            targetHeight = height1;
        } else {
            targetWidth = width2;
            targetHeight = height2;
        }
        
        if (width2 != targetWidth || height2 != targetHeight) {
            unsigned short int upSize = 1, downSize = 1;
            if (targetWidth > width2) {
                upSize = targetWidth / width2;
                if (upSize == 0) upSize = 1;
            } else {
                downSize = width2 / targetWidth;
                if (downSize == 0) downSize = 1;
            }
            
            if (!changeBmpSize(&R2, &G2, &B2, &width2, &height2, downSize, upSize)) {
                executeError(file1, file2, "Failed to resize second image");
            }
        }
        
        if (width1 != targetWidth || height1 != targetHeight) {
            unsigned short int upSize = 1, downSize = 1;
            if (targetWidth > width1) {
                upSize = targetWidth / width1;
                if (upSize == 0) upSize = 1;
            } else {
                downSize = width1 / targetWidth;
                if (downSize == 0) downSize = 1;
            }
            
            if (!changeBmpSize(&R1, &G1, &B1, &width1, &height1, downSize, upSize)) {
                executeError(file1, file2, "Failed to resize first image");
            }
        }
        
        double sumSqDiffR = 0, sumSqDiffG = 0, sumSqDiffB = 0;
        unsigned long long totalPixels = targetWidth * targetHeight;
        
        for (unsigned long long i = 0; i < targetHeight; i++) {
            for (unsigned long long j = 0; j < targetWidth; j++) {
                unsigned long long index = i * targetWidth + j;
                
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
        
        printf("Comparison Results:\n");
        printf("MSE R channel: %.2f\n", mseR);
        printf("MSE G channel: %.2f\n", mseG);
        printf("MSE B channel: %.2f\n", mseB);
        printf("Overall MSE: %.2f\n", mseTotalRGB);
        printf("PSNR: %.2f dB\n", psnr);
        printf("Similarity: %.2f%%\n", similarityPercent);
        
    } else {
        struct yuvFormat yuvFmt;
        initYuvFormat(&yuvFmt, format);
        
        if (!readYuvFile(file1, &Y1, &Cb1, &Cr1, width1, height1, &yuvFmt)) {
            executeError(file1, file2, "Failed to read first YUV data");
        }
        if (!readYuvFile(file2, &Y2, &Cb2, &Cr2, width2, height2, &yuvFmt)) {
            executeError(file1, file2, "Failed to read second YUV data");
        }
        
        unsigned long long targetWidth, targetHeight;
        
        if (width1 * height1 < width2 * height2) {
            targetWidth = width1;
            targetHeight = height1;
        } else {
            targetWidth = width2;
            targetHeight = height2;
        }
        if (width1 != targetWidth || height1 != targetHeight) {
            unsigned short int upSize = 1, downSize = 1;
            if (targetWidth > width1) {
                upSize = targetWidth / width1;
                if (upSize == 0) upSize = 1;
            } else {
                downSize = width1 / targetWidth;
                if (downSize == 0) downSize = 1;
            }
            
            if (!changeYuvSize(&Y1, &Cb1, &Cr1, &width1, &height1, downSize, upSize, &yuvFmt)) {
                executeError(file1, file2, "Failed to resize first YUV image");
            }
        }
        
        if (width2 != targetWidth || height2 != targetHeight) {

            unsigned short int upSize = 1, downSize = 1;
            if (targetWidth > width2) {
                upSize = targetWidth / width2;
                if (upSize == 0) upSize = 1;
            } else {
                downSize = width2 / targetWidth;
                if (downSize == 0) downSize = 1;
            }
            
            if (!changeYuvSize(&Y2, &Cb2, &Cr2, &width2, &height2, downSize, upSize, &yuvFmt)) {
                executeError(file1, file2, "Failed to resize second YUV image");
            }
        }
        
        double sumSqDiffY = 0, sumSqDiffCb = 0, sumSqDiffCr = 0;
        unsigned long long totalPixels = targetWidth * targetHeight;
        unsigned long long uvW = (targetWidth + yuvFmt.wMultiplier - 1) / yuvFmt.wMultiplier;
        unsigned long long uvH = (targetHeight + yuvFmt.hMultiplier - 1) / yuvFmt.hMultiplier;
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
        
        printf("Comparison Results:\n");
        printf("MSE Y channel: %.2f\n", mseY);
        printf("MSE Cb channel: %.2f\n", mseCb);
        printf("MSE Cr channel: %.2f\n", mseCr);
        printf("Overall MSE: %.2f\n", mseTotalYUV);
        printf("PSNR: %.2f dB\n", psnr);
        printf("Similarity: %.2f%%\n", similarityPercent);
    }
}