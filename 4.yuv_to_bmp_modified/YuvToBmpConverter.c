#include "converter.h"

int main(int argc, char **argv) {
    initMemoryManager();
    unsigned long long width = 0, height = 0;
    unsigned long long width2 = 0, height2 = 0;
    unsigned short int upSizeK = 1, downSizeK = 1;
    uint8_t isGrayScale = 0, isSizeChanged = 0, isCompareMode = 0;
    char *input_filename = NULL, *output_filename = NULL;
    FILE *inputFile = NULL, *outputFile = NULL;
    char *input_format = NULL, *output_format = NULL;
    uint8_t widthSpecified = 0, heightSpecified = 0;
    uint8_t width2Specified = 0, height2Specified = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--compare") == 0) {
            isCompareMode = 1;
            if (i + 2 >= argc) {
                executeError(NULL, NULL, "Compare mode requires two file arguments");
            }
            input_filename = argv[i + 1];
            output_filename = argv[i + 2];
            i += 2;
            
            if (i + 1 < argc && strcmp(argv[i + 1], "--format") == 0) {
                if (i + 2 >= argc) {
                    executeError(NULL, NULL, "Format must be specified after --format");
                }
                input_format = argv[i + 2];
                if (!isValidFormat(input_format)) {
                    executeError(NULL, NULL, "Invalid format specified for comparison");
                }
                i += 2;
            } else {
                char *ext1 = strrchr(input_filename, '.');
                char *ext2 = strrchr(output_filename, '.');
                if (ext1 && ext2) {
                    if (strcmpi(ext1, ".bmp") == 0 && strcmpi(ext2, ".bmp") == 0) {
                        input_format = "bmp";
                    } else if ((strcmpi(ext1, ".yuv") == 0) && 
                              (strcmpi(ext2, ".yuv") == 0)) {
                        input_format = "yuv420";
                    } else {
                        executeError(NULL, NULL, "Cannot compare different file types. Both files must be either BMP or YUV");
                    }
                } else {
                    executeError(NULL, NULL, "Cannot determine file format. Please use --format option.");
                }
            }
            
            while (i + 1 < argc) {
                if (strcmp(argv[i + 1], "--width") == 0) {
                    if (i + 2 >= argc || !isValidNumber(argv[i + 2])) {
                        executeError(NULL, NULL, "Width must be a positive number");
                    }
                    
                    width = strtoul(argv[i + 2], NULL, 10);
                    if (width == 0) {
                        executeError(NULL, NULL, "Width must be greater than zero");
                    }
                    widthSpecified = 1;
                    
                    if (i + 3 < argc && isValidNumber(argv[i + 3])) {
                        width2 = strtoul(argv[i + 3], NULL, 10);
                        if (width2 == 0) {
                            executeError(NULL, NULL, "Second width must be greater than zero");
                        }
                        width2Specified = 1;
                        i += 3;
                    } else {
                        width2 = width;
                        width2Specified = 1;
                        i += 2;
                    }
                } else if (strcmp(argv[i + 1], "--height") == 0) {
                    if (i + 2 >= argc || !isValidNumber(argv[i + 2])) {
                        executeError(NULL, NULL, "Height must be a positive number");
                    }
                    
                    height = strtoul(argv[i + 2], NULL, 10);
                    if (height == 0) {
                        executeError(NULL, NULL, "Height must be greater than zero");
                    }
                    heightSpecified = 1;
                    
                    if (i + 3 < argc && isValidNumber(argv[i + 3])) {
                        height2 = strtoul(argv[i + 3], NULL, 10);
                        if (height2 == 0) {
                            executeError(NULL, NULL, "Second height must be greater than zero");
                        }
                        height2Specified = 1;
                        i += 3;
                    } else {
                        height2 = height;
                        height2Specified = 1;
                        i += 2;
                    }
                } else {
                    break;
                }
            }
            
            inputFile = fopen(input_filename, "rb");
            if (!inputFile) {
                executeError(NULL, NULL, "Cannot open first file for comparison");
            }
            
            outputFile = fopen(output_filename, "rb"); 
            if (!outputFile) {
                executeError(inputFile, NULL, "Cannot open second file for comparison");
            }
            
            if (strcmp(input_format, "bmp") != 0) {
                if (!widthSpecified || !heightSpecified) {
                    executeError(inputFile, outputFile, "Width and height must be specified for first YUV file");
                }
                if (!width2Specified || !height2Specified) {
                    width2 = width;
                    height2 = height;
                }
                
                struct yuvFormat yuvFmt;
                initYuvFormat(&yuvFmt, input_format);
                
                unsigned long long fileSize1 = getFileSize(inputFile);
                unsigned long long expectedSize1 = width * height;
                if (yuvFmt.format == 444) {
                    expectedSize1 *= 3;
                } else if (yuvFmt.format == 422) {
                    expectedSize1 = expectedSize1 + expectedSize1/yuvFmt.uvRatio;
                } else if (yuvFmt.format == 420) {
                    expectedSize1 = expectedSize1 + expectedSize1/yuvFmt.uvRatio;
                }
                
                unsigned long long fileSize2 = getFileSize(outputFile);
                unsigned long long expectedSize2 = width2 * height2;
                if (yuvFmt.format == 444) {
                    expectedSize2 *= 3;
                } else if (yuvFmt.format == 422) {
                    expectedSize2 = expectedSize2 + expectedSize2/yuvFmt.uvRatio;
                } else if (yuvFmt.format == 420) {
                    expectedSize2 = expectedSize2 + expectedSize2/yuvFmt.uvRatio;
                }
                
                if (fileSize1 != expectedSize1) {
                    printf("Warning: First file size (%llu) doesn't match expected size (%llu) for dimensions %llux%llu\n", 
                           fileSize1, expectedSize1, width, height);
                }
                
                if (fileSize2 != expectedSize2) {
                    printf("Warning: Second file size (%llu) doesn't match expected size (%llu) for dimensions %llux%llu\n", 
                           fileSize2, expectedSize2, width2, height2);
                }
            }
            
            compareImagesWithDifferentSizes(inputFile, outputFile, input_format, width, height, width2, height2);
            fclose(inputFile);
            fclose(outputFile);
            freeAllMemory();
            return EXIT_SUCCESS;
        }
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --input parameter");
            }
            input_filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --output parameter");
            }
            output_filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--input-format") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --input-format parameter");
            }
            input_format = argv[i + 1];
            if (!isValidFormat(input_format)) {
                executeError(NULL, NULL, "Invalid input format");
            }
            i++;
        } else if (strcmp(argv[i], "--output-format") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --output-format parameter");
            }
            output_format = argv[i + 1];
            if (!isValidFormat(output_format)) {
                executeError(NULL, NULL, "Invalid output format");
            }
            i++;
        } else if (strcmp(argv[i], "--width") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --width parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Width must be a positive number");
            }
            width = strtoul(argv[i + 1], NULL, 10);
            if (width == 0) {
                executeError(NULL, NULL, "Width must be greater than zero");
            }
            widthSpecified = 1;
            i++;
        } else if (strcmp(argv[i], "--height") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --height parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Height must be a positive number");
            }
            height = strtoul(argv[i + 1], NULL, 10);
            if (height == 0) {
                executeError(NULL, NULL, "Height must be greater than zero");
            }
            heightSpecified = 1;
            i++;
        } else if (strcmp(argv[i], "--up-size") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --up-size parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Up-size factor must be a positive number");
            }
            upSizeK = strtoul(argv[i + 1], NULL, 10);
            if (upSizeK == 0) {
                executeError(NULL, NULL, "Up-size factor must be greater than zero");
            }
            isSizeChanged = 1;
            i++;
        } else if (strcmp(argv[i], "--down-size") == 0) {
            if (i + 1 >= argc) {
                executeError(NULL, NULL, "Missing value for --down-size parameter");
            }
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Down-size factor must be a positive number");
            }
            downSizeK = strtoul(argv[i + 1], NULL, 10);
            if (downSizeK == 0) {
                executeError(NULL, NULL, "Down-size factor must be greater than zero");
            }
            isSizeChanged = 1;
            i++;
        } else if (strcmp(argv[i], "--to-gray-scale") == 0) {
            isGrayScale = 1;
        }  else {
            char errorMsg[100];
            sprintf(errorMsg, "Unknown parameter: %s", argv[i]);
            executeError(NULL, NULL, errorMsg);
        }
    }
    
    if (input_filename == NULL || output_filename == NULL) {
        executeError(NULL, NULL, "Both input and output files must be specified");
    }
    
    if (input_format == NULL || output_format == NULL) {
        executeError(NULL, NULL, "Both input and output formats must be specified");
    }
    
    if (strcmp(input_format, "bmp") == 0) {
        if (widthSpecified || heightSpecified) {
            executeError(NULL, NULL, "Width and height should not be specified for BMP input (determined from header)");
        }
    } else {
        if (!widthSpecified || !heightSpecified) {
            executeError(NULL, NULL, "Width and height must be specified for YUV input");
        }
    }
    inputFile = fopen(input_filename, "rb");
    if (!inputFile) {
        executeError(NULL, NULL, "Cannot open input file");
    }
    outputFile = fopen(output_filename, "wb");
    if (!outputFile) {
        executeError(inputFile, NULL, "Cannot create output file");
    }

    setvbuf(inputFile, NULL, _IOFBF, 65536);
    setvbuf(outputFile, NULL, _IOFBF, 65536);
    
    if (strcmp(input_format, "bmp") == 0) {
        uint32_t pixelOffset;
        uint8_t *R = NULL, *G = NULL, *B = NULL;
        if (!readBmpHeader(inputFile, &width, &height, &pixelOffset)) {
            executeError(inputFile, outputFile, "Failed to read BMP header");
        }
        if (!readBmpPixelData(inputFile, &R, &G, &B, width, height, pixelOffset)) {
            executeError(inputFile, outputFile, "Failed to read BMP pixel data");
        }
        if (isSizeChanged) {
            if (!changeBmpSize(&R, &G, &B, &width, &height, downSizeK, upSizeK)) {
                executeError(inputFile, outputFile, "Failed to resize image");
            }
        }
        if (isGrayScale) {
            toGrayScaleBmp(R, G, B, &width, &height);
        }
        if (strcmp(output_format, "bmp") == 0) {
            if (!writeBmpToFile(outputFile, R, G, B, width, height)) {
                executeError(inputFile, outputFile, "Failed to write BMP data to output file");
            }
        } else {
            struct yuvFormat yOutputFormat = {1, 1, 1};
            uint8_t *Y = NULL, *Cb = NULL, *Cr = NULL;
            initYuvFormat(&yOutputFormat, output_format);
            if (!convertBmpToYuv(R, G, B, &Y, &Cb, &Cr, width, height, &yOutputFormat)) {
                executeError(inputFile, outputFile, "Failed to convert BMP to YUV");
            }
            if (!writeYuvToFile(outputFile, Y, Cb, Cr, width, height, &yOutputFormat)) {
                executeError(inputFile, outputFile, "Failed to write YUV data to output file");
            }
        }
    } else {
        struct yuvFormat yInputFormat = {1, 1, 1};
        uint8_t *Y = NULL, *Cb = NULL, *Cr = NULL;
        unsigned long long yuvFileSize = getFileSize(inputFile);
        if (yuvFileSize == 0) { 
            executeError(inputFile, outputFile, "Failed to get input file size"); 
        }
        if (!getYuvType(width, height, yuvFileSize, &yInputFormat)) {
            executeError(inputFile, outputFile, "Invalid YUV format or dimensions"); 
        }
        if (!readYuvFile(inputFile, &Y, &Cb, &Cr, width, height, &yInputFormat)) {
            executeError(inputFile, outputFile, "Failed to read YUV data");
        }
        if (isSizeChanged) {
            if (!changeYuvSize(&Y, &Cb, &Cr, &width, &height, downSizeK, upSizeK, &yInputFormat)) {
                executeError(inputFile, outputFile, "Failed to resize image");
            }
        }
        if (isGrayScale) {
            toGrayScaleYuv(Cb, Cr, &width, &height, &yInputFormat);
        }
        if (strcmp(output_format, "bmp") == 0) {
            uint8_t *R = NULL, *G = NULL, *B = NULL;
            if (!convertYuvToBmp(Y, Cb, Cr, &R, &G, &B, width, height, &yInputFormat)) {
                executeError(inputFile, outputFile, "Failed to convert YUV to BMP");
            }
            if (!writeBmpToFile(outputFile, R, G, B, width, height)) {
                executeError(inputFile, outputFile, "Failed to write BMP data to output file");
            }
        } else {
            struct yuvFormat yOutputFormat = {1, 1, 1};
            uint8_t *outY = NULL, *outCb = NULL, *outCr = NULL;
            initYuvFormat(&yOutputFormat, output_format);
            if (!convertYuvToYuv(Y, Cb, Cr, &outY, &outCb, &outCr, width, height, &yInputFormat, &yOutputFormat)) {
                executeError(inputFile, outputFile, "Failed to convert between YUV formats");
            }
            
            if (!writeYuvToFile(outputFile, outY, outCb, outCr, width, height, &yOutputFormat)) {
                executeError(inputFile, outputFile, "Failed to write YUV data to output file");
            }
        }
    }
    fclose(inputFile);
    fclose(outputFile);
    freeAllMemory();
    return EXIT_SUCCESS;
}
