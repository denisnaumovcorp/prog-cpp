#ifndef CONVERTERSTRUCTURES_H
#define CONVERTERSTRUCTURES_H

#include "converterLibs.h"

#pragma pack(push, 2)  
struct BmpHeader {
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

struct YuvFormat {
    unsigned short int format;      
    uint8_t wMultiplier;            
    uint8_t hMultiplier;            
    uint8_t uvRatio;                
};
#pragma pack(pop)

extern const std::vector<std::string> VALID_FORMATS;

#endif // CONVERTERSTRUCTURES_H