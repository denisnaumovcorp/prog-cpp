#include "converter.h"

int main(int argc, char **argv) {
    unsigned long long width = 0, height = 0;
    unsigned long long width2 = 0, height2 = 0;
    unsigned short int upSizeK = 1, downSizeK = 1;
    bool isGrayScale = false, isSizeChanged = false, isCompareMode = false;
    char *input_filename = nullptr, *output_filename = nullptr;
    std::ifstream inputFile;
    std::ofstream outputFile;
    char *input_format = nullptr, *output_format = nullptr;
    bool widthSpecified = false, heightSpecified = false;
    bool width2Specified = false, height2Specified = false;
    for (int i = 1; i < argc; i++) {
        if (_stricmp(argv[i], "--compare") == 0) {
            isCompareMode = true;
            if (i + 2 >= argc) {
                executeError(&inputFile, &outputFile, "Compare mode requires two file arguments");
            }
            input_filename = argv[i + 1];
            output_filename = argv[i + 2];
            i += 2;
            
            if (i + 1 < argc && _stricmp(argv[i + 1], "--format") == 0) {
                if (i + 2 >= argc) {
                    executeError(&inputFile, &outputFile, "Format must be specified after --format");
                }
                input_format = argv[i + 2];
                if (!isValidFormat(input_format)) {
                    executeError(&inputFile, &outputFile, "Invalid format specified for comparison");
                }
                i += 2;
            } else {
                char *ext1 = strrchr(input_filename, '.');
                char *ext2 = strrchr(output_filename, '.');
                if (ext1 && ext2) {
                    if (_stricmp(ext1, ".bmp") == 0 && _stricmp(ext2, ".bmp") == 0) {
                        input_format = (char*)"bmp";
                    } else if ((_stricmp(ext1, ".yuv") == 0) && 
                              (_stricmp(ext2, ".yuv") == 0)) {
                        input_format = (char*)"yuv420";
                    } else {
                        executeError(&inputFile, &outputFile, "Cannot compare different file types. Both files must be either BMP or YUV");
                    }
                } else {
                    executeError(&inputFile, &outputFile, "Cannot determine file format. Please use --format option.");
                }
            }
            
            while (i + 1 < argc) {
                if (_stricmp(argv[i + 1], "--width") == 0) {
                    if (i + 2 >= argc || !isValidNumber(argv[i + 2])) {
                        executeError(&inputFile, &outputFile, "Width must be a positive number");
                    }
                    
                    width = strtoul(argv[i + 2], NULL, 10);
                    if (width == 0) {
                        executeError(&inputFile, &outputFile, "Width must be greater than zero");
                    }
                    widthSpecified = true;
                    
                    if (i + 3 < argc && isValidNumber(argv[i + 3])) {
                        width2 = strtoul(argv[i + 3], NULL, 10);
                        if (width2 == 0) {
                            executeError(&inputFile, &outputFile, "Second width must be greater than zero");
                        }
                        width2Specified = true;
                        i += 3;
                    } else {
                        width2 = width;
                        width2Specified = true;
                        i += 2;
                    }
                } else if (_stricmp(argv[i + 1], "--height") == 0) {
                    if (i + 2 >= argc || !isValidNumber(argv[i + 2])) {
                        executeError(&inputFile, &outputFile, "Height must be a positive number");
                    }
                    
                    height = strtoul(argv[i + 2], NULL, 10);
                    if (height == 0) {
                        executeError(&inputFile, &outputFile, "Height must be greater than zero");
                    }
                    heightSpecified = true;
                    
                    if (i + 3 < argc && isValidNumber(argv[i + 3])) {
                        height2 = strtoul(argv[i + 3], NULL, 10);
                        if (height2 == 0) {
                            executeError(&inputFile, &outputFile, "Second height must be greater than zero");
                        }
                        height2Specified = true;
                        i += 3;
                    } else {
                        height2 = height;
                        height2Specified = true;
                        i += 2;
                    }
                } else {
                    break;
                }
            }
            
            inputFile.open(input_filename, std::ios::binary);
            if (!inputFile) {
                executeError(&inputFile, &outputFile, "Cannot open first file for comparison");
            }
            
            std::ifstream outputFileAsInput(output_filename, std::ios::binary);
            if (!outputFileAsInput) {
                inputFile.close();
                executeError(&inputFile, &outputFile, "Cannot open second file for comparison");
            }
            
            if (strcmp(input_format, "bmp") != 0) {
                if (!widthSpecified || !heightSpecified) {
                    inputFile.close();
                    outputFileAsInput.close();
                    executeError(&inputFile, &outputFile, "Width and height must be specified for first YUV file");
                }
                if (!width2Specified || !height2Specified) {
                    width2 = width;
                    height2 = height;
                }
                
                YuvFormat yuvFmt;
                initYuvFormat(yuvFmt, input_format);
                
                inputFile.seekg(0, std::ios::end);
                unsigned long long fileSize1 = inputFile.tellg();
                inputFile.seekg(0, std::ios::beg);
                
                unsigned long long expectedSize1 = width * height;
                if (yuvFmt.format == 444) {
                    expectedSize1 *= 3;
                } else if (yuvFmt.format == 422) {
                    expectedSize1 = expectedSize1 + expectedSize1/yuvFmt.uvRatio;
                } else if (yuvFmt.format == 420) {
                    expectedSize1 = expectedSize1 + expectedSize1/yuvFmt.uvRatio;
                }
                
                outputFileAsInput.seekg(0, std::ios::end);
                unsigned long long fileSize2 = outputFileAsInput.tellg();
                outputFileAsInput.seekg(0, std::ios::beg);
                
                unsigned long long expectedSize2 = width2 * height2;
                if (yuvFmt.format == 444) {
                    expectedSize2 *= 3;
                } else if (yuvFmt.format == 422) {
                    expectedSize2 = expectedSize2 + expectedSize2/yuvFmt.uvRatio;
                } else if (yuvFmt.format == 420) {
                    expectedSize2 = expectedSize2 + expectedSize2/yuvFmt.uvRatio;
                }
                
                if (fileSize1 != expectedSize1) {
                    std::cout << "Warning: First file size (" << fileSize1 << ") doesn't match expected size (" 
                              << expectedSize1 << ") for dimensions " << width << "x" << height << std::endl;
                }
                
                if (fileSize2 != expectedSize2) {
                    std::cout << "Warning: Second file size (" << fileSize2 << ") doesn't match expected size (" 
                              << expectedSize2 << ") for dimensions " << width2 << "x" << height2 << std::endl;
                }
            }
            
            compareImagesWithDifferentSizes(inputFile, outputFileAsInput, input_format, width, height, width2, height2);
            inputFile.close();
            outputFileAsInput.close();
            return EXIT_SUCCESS;
        }
    }
    
    for (int i = 1; i < argc; i++) {
        if (_stricmp(argv[i], "--input") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --input parameter");
            }
            input_filename = argv[i + 1];
            i++;
        } else if (_stricmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --output parameter");
            }
            output_filename = argv[i + 1];
            i++;
        } else if (_stricmp(argv[i], "--input-format") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --input-format parameter");
            }
            input_format = argv[i + 1];
            if (!isValidFormat(input_format)) {
                executeError(&inputFile, &outputFile, "Invalid input format");
            }
            i++;
        } else if (_stricmp(argv[i], "--output-format") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --output-format parameter");
            }
            output_format = argv[i + 1];
            if (!isValidFormat(output_format)) {
                executeError(&inputFile, &outputFile, "Invalid output format");
            }
            i++;
        } else if (_stricmp(argv[i], "--width") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --width parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(&inputFile, &outputFile, "Width must be a positive number");
            }
            width = strtoul(argv[i + 1], NULL, 10);
            if (width == 0) {
                executeError(&inputFile, &outputFile, "Width must be greater than zero");
            }
            widthSpecified = true;
            i++;
        } else if (_stricmp(argv[i], "--height") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --height parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(&inputFile, &outputFile, "Height must be a positive number");
            }
            height = strtoul(argv[i + 1], NULL, 10);
            if (height == 0) {
                executeError(&inputFile, &outputFile, "Height must be greater than zero");
            }
            heightSpecified = true;
            i++;
        } else if (_stricmp(argv[i], "--up-size") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --up-size parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(&inputFile, &outputFile, "Up-size factor must be a positive number");
            }
            upSizeK = strtoul(argv[i + 1], NULL, 10);
            if (upSizeK == 0) {
                executeError(&inputFile, &outputFile, "Up-size factor must be greater than zero");
            }
            isSizeChanged = true;
            i++;
        } else if (_stricmp(argv[i], "--down-size") == 0) {
            if (i + 1 >= argc) {
                executeError(&inputFile, &outputFile, "Missing value for --down-size parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(&inputFile, &outputFile, "Down-size factor must be a positive number");
            }
            downSizeK = strtoul(argv[i + 1], NULL, 10);
            if (downSizeK == 0) {
                executeError(&inputFile, &outputFile, "Down-size factor must be greater than zero");
            }
            isSizeChanged = true;
            i++;
        } else if (_stricmp(argv[i], "--to-gray-scale") == 0) {
            isGrayScale = true;
        } else {
            executeError(&inputFile, &outputFile, "Unknown parameter: " + std::string(argv[i]));
        }
    }
    
    if (input_filename == nullptr || output_filename == nullptr) {
        executeError(&inputFile, &outputFile, "Both input and output files must be specified");
    }
    
    if (input_format == nullptr || output_format == nullptr) {
        executeError(&inputFile, &outputFile, "Both input and output formats must be specified");
    }
    
    if (_stricmp(input_format, "bmp") == 0) {
        if (widthSpecified || heightSpecified) {
            executeError(&inputFile, &outputFile, "Width and height should not be specified for BMP input (determined from header)");
        }
    } else {
        if (!widthSpecified || !heightSpecified) {
            executeError(&inputFile, &outputFile, "Width and height must be specified for YUV input");
        }
    }
    
    inputFile.open(input_filename, std::ios::binary);
    if (!inputFile) {
        executeError(&inputFile, &outputFile, "Cannot open input file");
    }
    
    outputFile.open(output_filename, std::ios::binary);
    if (!outputFile) {
        executeError(&inputFile, &outputFile, "Cannot create output file");
    }

    if (_stricmp(input_format, "bmp") == 0) {
        uint32_t pixelOffset;
        std::vector<uint8_t> R, G, B;
        
        if (!readBmpHeader(inputFile, width, height, pixelOffset)) {
            executeError(&inputFile, &outputFile, "Failed to read BMP header");
        }
        
        if (!readBmpPixelData(inputFile, R, G, B, width, height, pixelOffset)) {
            executeError(&inputFile, &outputFile, "Failed to read BMP pixel data");
        }
        
        if (isSizeChanged) {
            if (!changeSize(R, G, B, width, height, downSizeK, upSizeK)) {
                executeError(&inputFile, &outputFile, "Failed to resize image");
            }
        }
        
        if (isGrayScale) {
            toGrayScale(R, G, B, width, height);
        }
        
        if (_stricmp(output_format, "bmp") == 0) {
            if (!writeBmpToFile(outputFile, R, G, B, width, height)) {
                executeError(&inputFile, &outputFile, "Failed to write BMP data to output file");
            }
        } else {
            YuvFormat yOutputFormat;
            std::vector<uint8_t> Y, Cb, Cr;
            
            initYuvFormat(yOutputFormat, output_format);
            
            if (!convertBmpToYuv(R, G, B, Y, Cb, Cr, width, height, yOutputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to convert BMP to YUV");
            }
            
            if (!writeYuvToFile(outputFile, Y, Cb, Cr, width, height, yOutputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to write YUV data to output file");
            }
        }
    } else {
        YuvFormat yInputFormat;
        std::vector<uint8_t> Y, Cb, Cr;
        
        inputFile.seekg(0, std::ios::end);
        unsigned long long yuvFileSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        
        if (yuvFileSize == 0) {
            executeError(&inputFile, &outputFile, "Failed to get input file size");
        }
        
        if (!getYuvType(width, height, yuvFileSize, yInputFormat)) {
            executeError(&inputFile, &outputFile, "Invalid YUV format or dimensions");
        }
        
        if (!readYuvFile(inputFile, Y, Cb, Cr, width, height, yInputFormat)) {
            executeError(&inputFile, &outputFile, "Failed to read YUV data");
        }
        
        if (isSizeChanged) {
            if (!changeSize(Y, Cb, Cr, width, height, downSizeK, upSizeK, yInputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to resize image");
            }
        }
        
        if (isGrayScale) {
            toGrayScale(Cb, Cr, width, height, yInputFormat);
        }
        
        if (_stricmp(output_format, "bmp") == 0) {
            std::vector<uint8_t> R, G, B;
            
            if (!convertYuvToBmp(Y, Cb, Cr, R, G, B, width, height, yInputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to convert YUV to BMP");
            }
            
            if (!writeBmpToFile(outputFile, R, G, B, width, height)) {
                executeError(&inputFile, &outputFile, "Failed to write BMP data to output file");
            }
        } else {
            YuvFormat yOutputFormat;
            std::vector<uint8_t> outY, outCb, outCr;
            initYuvFormat(yOutputFormat, output_format);
            if (!convertYuvToYuv(Y, Cb, Cr, outY, outCb, outCr, width, height, yInputFormat, yOutputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to convert between YUV formats");
            }
            
            if (!writeYuvToFile(outputFile, outY, outCb, outCr, width, height, yOutputFormat)) {
                executeError(&inputFile, &outputFile, "Failed to write YUV data to output file");
            }
        }
    }
    inputFile.close();
    outputFile.close();
    return EXIT_SUCCESS;
}
