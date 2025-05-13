#include "converterLibs.h"
#include "converterStructures.h"

const std::vector<std::string> VALID_FORMATS = {"yuv444", "yuv422", "yuv420", "bmp"};

void executeError(std::ifstream* inputFile, std::ofstream* outputFile, const std::string& errorText) {
    if (inputFile != nullptr && inputFile->is_open()) {
        inputFile->close();
    }
    if (outputFile != nullptr && outputFile->is_open()) {
        outputFile->close();
    }
    std::cerr << errorText << std::endl;
    exit(EXIT_FAILURE);
}

void executeError(std::ifstream* file1, std::ifstream* file2, const std::string& errorText) {
    if (file1 != nullptr && file1->is_open()) {
        file1->close();
    }
    if (file2 != nullptr && file2->is_open()) {
        file2->close();
    }
    std::cerr << errorText << std::endl;
    exit(EXIT_FAILURE);
}

void initYuvFormat(YuvFormat& yFormat, const std::string& format) {
    if (format == "yuv444") {
        yFormat.format = 444;
        yFormat.wMultiplier = 1;
        yFormat.hMultiplier = 1;
        yFormat.uvRatio = 1;
    } else if (format == "yuv422") {
        yFormat.format = 422;
        yFormat.wMultiplier = 2;
        yFormat.hMultiplier = 1;
        yFormat.uvRatio = 2;
    } else if (format == "yuv420") {
        yFormat.format = 420;
        yFormat.wMultiplier = 2;
        yFormat.hMultiplier = 2;
        yFormat.uvRatio = 4;
    }
}

bool getYuvType(unsigned long long width, unsigned long long height, 
                unsigned long long fileSize, YuvFormat& yFormat) {
    unsigned long long area = width * height;
    if (fileSize == area * 3) {
        yFormat.hMultiplier = 1;
        yFormat.wMultiplier = 1;
        yFormat.uvRatio = 1;
        yFormat.format = 444;
    } else if (fileSize == area + 2 * ((width + 1) / 2) * height) {
        yFormat.hMultiplier = 1;
        yFormat.wMultiplier = 2;
        yFormat.uvRatio = 2;
        yFormat.format = 422;
    } else if (fileSize == area + 2 * ((width + 1) / 2) * ((height + 1) / 2)) {
        yFormat.wMultiplier = 2;
        yFormat.hMultiplier = 2;
        yFormat.uvRatio = 4;
        yFormat.format = 420;
    } else {
        return false;
    }
    return true;
}

bool isValidNumber(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    return std::all_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });
}

bool isValidFormat(const std::string& format) {
    return std::find(VALID_FORMATS.begin(), VALID_FORMATS.end(), format) != VALID_FORMATS.end();
}

uint8_t bilinearInterpolate(const std::vector<uint8_t>& src, unsigned int srcWidth, unsigned int srcHeight, double x, double y) {
    int x1 = static_cast<int>(x);
    int y1 = static_cast<int>(y);
    int x2 = (x1 + 1 < srcWidth) ? x1 + 1 : x1;
    int y2 = (y1 + 1 < srcHeight) ? y1 + 1 : y1;
    double dx = x - x1;
    double dy = y - y1;
    
    uint8_t q11 = src[y1 * srcWidth + x1];
    uint8_t q12 = src[y2 * srcWidth + x1];
    uint8_t q21 = src[y1 * srcWidth + x2];
    uint8_t q22 = src[y2 * srcWidth + x2];
    
    double result = q11 * (1 - dx) * (1 - dy) +
                    q21 * dx * (1 - dy) +
                    q12 * (1 - dx) * dy +
                    q22 * dx * dy;
                    
    return static_cast<uint8_t>(std::round(result));
}