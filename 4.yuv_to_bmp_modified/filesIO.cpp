#include "converterLibs.h"
#include "converterStructures.h"

bool setBmpHeader(std::ofstream& file, unsigned int width, unsigned int height) {
    BmpHeader header;
    header.type = 0x4D42;
    header.size = 54 + (width * 3 + (4 - (width * 3) % 4) % 4) * height;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offset = 54;
    header.dib_size = 40;
    header.width = width;
    header.height = height;
    header.planes = 1;
    header.bpp = 24;
    header.compression = 0;
    header.image_size = 0;
    header.x_ppm = 0;
    header.y_ppm = 0;   
    header.num_colors = 0;
    header.important_colors = 0;
    
    file.write(reinterpret_cast<const char*>(&header), 54);
    if (file.fail()) {
        return false;
    }
    return true;
}

bool readBmpHeader(std::ifstream& file, unsigned long long& width, unsigned long long& height, uint32_t& pixelOffset) {
    uint32_t fileSize;
    
    file.seekg(2, std::ios::beg);
    file.read(reinterpret_cast<char*>(&fileSize), 4);
    if (file.fail()) {
        return false;
    }
    
    file.seekg(10, std::ios::beg);
    file.read(reinterpret_cast<char*>(&pixelOffset), 4);
    if (file.fail()) {
        return false;
    }
    
    uint32_t width32, height32;
    file.seekg(18, std::ios::beg);
    file.read(reinterpret_cast<char*>(&width32), 4);
    if (file.fail()) {
        return false;
    }
    
    file.read(reinterpret_cast<char*>(&height32), 4);
    if (file.fail()) {
        return false;
    }
    
    width = width32;
    height = height32;
    return true;
}

bool readBmpPixelData(std::ifstream& file, std::vector<uint8_t>& R, std::vector<uint8_t>& G, 
                     std::vector<uint8_t>& B, unsigned long long width, unsigned long long height, uint32_t pixelOffset) {
    unsigned long long rgbSize = width * height;
    int padding = (4 - ((width * 3) % 4)) % 4;
    
    file.seekg(pixelOffset, std::ios::beg);
    
    R.resize(rgbSize);
    G.resize(rgbSize);
    B.resize(rgbSize);
    
    std::vector<uint8_t> paddingBytes(padding);
    
    for (int i = height - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < width; j++) {
            uint8_t b, g, r;
            file.read(reinterpret_cast<char*>(&b), 1);
            file.read(reinterpret_cast<char*>(&g), 1);
            file.read(reinterpret_cast<char*>(&r), 1);
            
            if (file.fail()) {
                return false;
            }
            
            B[i * width + j] = b;
            G[i * width + j] = g;
            R[i * width + j] = r;
        }
        
        file.read(reinterpret_cast<char*>(paddingBytes.data()), padding);
    }
    
    return true;
}

bool writeBmpToFile(std::ofstream& file, const std::vector<uint8_t>& R, const std::vector<uint8_t>& G, 
                   const std::vector<uint8_t>& B, unsigned long long width, unsigned long long height) {
    int padding = (4 - ((width * 3) % 4)) % 4;
    
    if (!setBmpHeader(file, width, height)) {
        return false;
    }
    
    std::vector<uint8_t> paddingBytes(padding, 0);
    
    for (int i = height - 1; i >= 0; i--) {
        for (unsigned long long j = 0; j < width; j++) {
            file.put(B[i * width + j]);
            file.put(G[i * width + j]);
            file.put(R[i * width + j]);
            
            if (file.fail()) {
                return false;
            }
        }
        
        file.write(reinterpret_cast<const char*>(paddingBytes.data()), padding);
        if (file.fail()) {
            return false;
        }
    }
    
    return true;
}

bool readYuvFile(std::ifstream& file, std::vector<uint8_t>& Y, std::vector<uint8_t>& Cb, 
                std::vector<uint8_t>& Cr, unsigned long long width, unsigned long long height, 
                const YuvFormat& format) {
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format.wMultiplier - 1) / format.wMultiplier;
    unsigned long long uvH = (height + format.hMultiplier - 1) / format.hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    
    Y.resize(ySize);
    Cb.resize(chromaSize);
    Cr.resize(chromaSize);
    
    file.read(reinterpret_cast<char*>(Y.data()), ySize);
    if (file.fail()) {
        return false;
    }
    
    file.read(reinterpret_cast<char*>(Cb.data()), chromaSize);
    if (file.fail()) {
        return false;
    }
    
    file.read(reinterpret_cast<char*>(Cr.data()), chromaSize);
    if (file.fail()) {
        return false;
    }
    
    return true;
}

bool writeYuvToFile(std::ofstream& file, const std::vector<uint8_t>& Y, const std::vector<uint8_t>& Cb, 
                   const std::vector<uint8_t>& Cr, unsigned long long width, unsigned long long height, 
                   const YuvFormat& format) {
    unsigned long long ySize = width * height;
    unsigned long long uvW = (width + format.wMultiplier - 1) / format.wMultiplier;
    unsigned long long uvH = (height + format.hMultiplier - 1) / format.hMultiplier;
    unsigned long long chromaSize = uvW * uvH;
    
    file.write(reinterpret_cast<const char*>(Y.data()), ySize);
    if (file.fail()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(Cb.data()), chromaSize);
    if (file.fail()) {
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(Cr.data()), chromaSize);
    if (file.fail()) {
        return false;
    }
    
    return true;
}