#include <stdio.h>
#include <stdint.h>

unsigned long long getFileSize(FILE* f) {
    unsigned long long size = 0;
    fseek(f, 0, SEEK_END);
    size = ftello(f);
    rewind(f);
    return size;
}

unsigned short int getYuvType(unsigned int area, unsigned long long fileSize) {
    if (area * 3 == fileSize) {
        return 444;
    } else if (area * 2 == fileSize) {
        return 422;
    } else {
        return 420;
    }
}

void setBmpHeader(FILE* f, unsigned int width, unsigned int  height) {
    #pragma pack(push, 1)  
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
        .compression = 0x0,
        .image_size = 0x0,
        .x_ppm = 0x0,
        .y_ppm = 0x0,   
        .num_colors = 0x0,
        .important_colors = 0x0
    };
    fwrite(&header, 54, 1, f);
}

unsigned char bilinearInterpolate(unsigned char* src, int srcWidth, int srcHeight, double x, double y) {
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