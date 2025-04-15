#include "converter.h"

int main(int argc, char **argv) {
    unsigned int width = 0, height = 0;
    struct yuvFormat yFormat = {1, 1, 1};
    unsigned short int isGrayScale = 0, upSizeK = 1, downSizeK = 1, isSizeChanged = 0;
    long long i, j;
    int opt, index = 0;
    char *input_filename = NULL, *output_filename = NULL;
    FILE *inputFile = NULL, *outputFile = NULL;
    if (argc < 5) {
        perror("Usage: ––input <input> ––output <output> ––width <width> ––height <height> [options]\n");
        return EXIT_FAILURE;
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--input") == 0) {
            input_filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--output") == 0) {
            output_filename = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--width") == 0) {
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Invalid width - must contain only digits");
            }
            width = strtoul(argv[i + 1], NULL, 10);
            if (width == 0) {
                executeError(NULL, NULL, "Invalid width - must be a natural number");
            }
            i++;
        } else if (strcmp(argv[i], "--height") == 0) {
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Invalid height - must contain only digits");
            }
            height = strtoul(argv[i + 1], NULL, 10);
            if (height == 0) {
                executeError(NULL, NULL, "Invalid height - must be a natural number");
            }
            i++;
        } else if (strcmp(argv[i], "--up-size") == 0) {
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Invalid up-size coefficient - must contain only digits");
            }
            upSizeK = strtoul(argv[i + 1], NULL, 10);
            if (upSizeK == 0) {
                executeError(NULL, NULL, "Invalid up-size coefficient - must be natural number");
            }
            isSizeChanged = 1;
            i++;
        } else if (strcmp(argv[i], "--down-size") == 0) {
            if (!isValidNumber(argv[i + 1])) {
                executeError(NULL, NULL, "Invalid down-size coefficient - must contain only digits");
            }
            downSizeK = strtoul(argv[i + 1], NULL, 10);
            if (downSizeK == 0) {
                executeError(NULL, NULL, "Invalid down-size coefficient - must be natural number");
            }
            isSizeChanged = 1;
            i++;
        } else if (strcmp(argv[i], "--to-gray-scale") == 0) {
            isGrayScale = 1;
        } else {
            executeError(NULL, NULL, "Invalid option");
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

    unsigned long long yuvFileSize = getFileSize(inputFile);
    if (yuvFileSize == 0) {
       executeError(inputFile, outputFile, "An error occurred while reading the input file. Perhaps the file is corrupted or the programme does not have access to it"); 
    }
    getYuvType(width * height, yuvFileSize, &yFormat);
    if (yFormat.format == 0) {
        executeError(inputFile, outputFile, "Your yuv file has an incorrect format and/or is corrupted"); 
    }
    unsigned long long ySize = width * height, cbSize = ySize / yFormat.uvRatio, crSize = cbSize;
    uint8_t *y = (uint8_t *)malloc(ySize);
    if (y == NULL) {
        executeError(inputFile, outputFile, "You do not have enough memory or it is not available. Buy more"); 
    }
    uint8_t *cb = (uint8_t *)malloc(cbSize);
    if (cb == NULL) {
        free(y);
        executeError(inputFile, outputFile, "You do not have enough memory or it is not available. Buy more"); 
    }
    uint8_t *cr = (uint8_t *)malloc(crSize);
    if (cr == NULL) {
        free(y);
        free(cb);
        executeError(inputFile, outputFile, "You do not have enough memory or it is not available. Buy more"); 
    }
    if (fread(y, 1, ySize, inputFile) == 0) {
        free(y);
        free(cb);
        free(cr);
        executeError(inputFile, outputFile, "An error occurred while reading the input file. Perhaps the file is corrupted or the programme does not have access to it"); 
    }
    if (fread(cb, 1, cbSize, inputFile) == 0) {
        free(y);
        free(cb);
        free(cr);
        executeError(inputFile, outputFile, "An error occurred while reading the input file. Perhaps the file is corrupted or the programme does not have access to it"); 
    }
    if (fread(cr, 1, crSize, inputFile) == 0) {
        free(y);
        free(cb);
        free(cr);
        executeError(inputFile, outputFile, "An error occurred while reading the input file. Perhaps the file is corrupted or the programme does not have access to it"); 
    }
    if (isSizeChanged) {
        if (changeSize(&y, &cb, &cr, &width, &height, downSizeK, upSizeK, &yFormat) == 0) {
            free(y);
            free(cb);
            free(cr);
            executeError(inputFile, outputFile, "An error occurred while changing the size of the image. Perhaps the file is corrupted or the programme does not have access to it");
        }
    }
    if (isGrayScale) {
        toGrayScale(cb, cr, &width, &height, &yFormat);
    }
    if (setBmpHeader(outputFile, width, height) == 0) {
        executeError(inputFile, outputFile, "An error occurred while writing data to the output file. Perhaps the file is corrupted or the programme does not have access to it");
    }
    unsigned short int bmpPadding = (4 - (width * 3) % 4) % 4;
    uint8_t rgb[3];
    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            unsigned int chromaIndex = (i / yFormat.hMultiplier) * (width / yFormat.wMultiplier) + (j / yFormat.wMultiplier);
            short int rVal = y[i * width + j] + 1.4 * (cr[chromaIndex] - 128);
            short int gVal = y[i * width + j] - 0.343 * (cb[chromaIndex] - 128) - 0.711 * (cr[chromaIndex] - 128);
            short int bVal = y[i * width + j] + 1.762 * (cb[chromaIndex] - 128);
            rgb[2] = (rVal < 0) ? 0 : ((rVal > 255) ? 255 : rVal);
            rgb[1] = (gVal < 0) ? 0 : ((gVal > 255) ? 255 : gVal);
            rgb[0] = (bVal < 0) ? 0 : ((bVal > 255) ? 255 : bVal);
            if (fwrite(rgb, 1, 3, outputFile) == 0) {
                free(y);
                free(cb);
                free(cr);
                executeError(inputFile, outputFile, "An error occurred while writing data to the output file. Perhaps the file is corrupted or the programme does not have access to it");
            }
        }
        for (int p = 0; p < bmpPadding; p++) {
            putc(0, outputFile);
        }
        if (ferror(outputFile) != 0) {
            free(y);
            free(cb);
            free(cr);
            executeError(inputFile, outputFile, "An error occurred while writing data to the output file. Perhaps the file is corrupted or the programme does not have access to it");
        }
    }
    free(y);
    free(cb);
    free(cr);
    fclose(inputFile);
    fclose(outputFile);
    return EXIT_SUCCESS;
}
