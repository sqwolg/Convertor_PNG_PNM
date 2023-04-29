#include "return_codes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <zlib.h>
#include <libdeflate.h>

#if defined(ZLIB)
#include <zlib.h>
#elif defined(LIBDEFLATE)
#include <libdeflate.h>
#elif defined(ISAL)
#include <include/igzip_lib.h>
#else
#error "Defined library is not supported"
#endif

#define BUFFER_SIZE 8
#define SMALL_BUFFER_SIZE 4
#define FILE_PATH "/Users/sqwolg/Downloads/Telegram Desktop/png_colortype3_64colors.png"

static int width;
static int height;
static int colourType;
static int sizeOfData = 0;
static int sizeOfPalette = 0;
static int sizeOfDataDecrypt;

int isPNG(FILE *file)
{
    unsigned char buffer[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    unsigned char readBuffer[8];
    size_t bytesRead = fread(readBuffer, 1, BUFFER_SIZE, file);
    if (bytesRead != BUFFER_SIZE || memcmp(readBuffer, buffer, BUFFER_SIZE) != 0)
    {
        return ERROR_DATA_INVALID;
    }
    return 0;
}

int bytesToInt(unsigned char const *arr, int start, int length)
{
    int result = arr[start] << (length - 1) * 8;
    for (int i = length - 2; i >= 0; i--)
    {
        result = result | (arr[start + length - i - 1] << i * 8);
    }
    return result;
}

int readIHDR(FILE *file)
{
    unsigned char const IHDR[4] = { 0x49, 0x48, 0x44, 0x52 };
    unsigned char sizeOfChunk[4];
    unsigned char nameOfChunk[4];
    unsigned char inChunk[13];
    unsigned char controlSum[4];

    size_t bytesRead = fread(sizeOfChunk, 1, SMALL_BUFFER_SIZE, file);
    if ((bytesRead != SMALL_BUFFER_SIZE) || (bytesToInt(sizeOfChunk, 0, SMALL_BUFFER_SIZE) != 13))
    {
        return ERROR_DATA_INVALID;
    }
    printf("%x %x %x %x\n", sizeOfChunk[0], sizeOfChunk[1], sizeOfChunk[2], sizeOfChunk[3]);

    bytesRead = fread(nameOfChunk, 1, SMALL_BUFFER_SIZE, file);
    if ((bytesRead != SMALL_BUFFER_SIZE) || (memcmp(nameOfChunk, IHDR, SMALL_BUFFER_SIZE) != 0))
    {
        return ERROR_DATA_INVALID;
    }
    printf("%x %x %x %x\n", nameOfChunk[0], nameOfChunk[1], nameOfChunk[2], nameOfChunk[3]);

    bytesRead = fread(inChunk, 1, 13, file);
    if (bytesRead != 13)
    {
        return ERROR_DATA_INVALID;
    }
    for (int i = 0; i < 13; i++)
    {
        printf("%x ", inChunk[i]);
    }
    printf("\n");

    width = bytesToInt(inChunk, 0, 4);
    height = bytesToInt(inChunk, 4, 4);
    colourType = bytesToInt(inChunk, 9, 1);
    fread(controlSum, 1, SMALL_BUFFER_SIZE, file);
    printf("%x %x %x %x\n", controlSum[0], controlSum[1], controlSum[2], controlSum[3]);
    printf("\n");
    return 0;
}

int readIEND(FILE *file)
{
    unsigned char const IEND[4] = { 0x49, 0x45, 0x4E, 0x44 };
    unsigned char sizeOfChunk[4];
    unsigned char nameOfChunk[4];
    unsigned char controlSum[4];

    size_t bytesRead = fread(sizeOfChunk, 1, SMALL_BUFFER_SIZE, file);
    if ((bytesRead != SMALL_BUFFER_SIZE) || (bytesToInt(sizeOfChunk, 0, SMALL_BUFFER_SIZE) != 0))
    {
        return ERROR_DATA_INVALID;
    }
    printf("%x %x %x %x\n", sizeOfChunk[0], sizeOfChunk[1], sizeOfChunk[2], sizeOfChunk[3]);

    bytesRead = fread(nameOfChunk, 1, SMALL_BUFFER_SIZE, file);
    if ((bytesRead != SMALL_BUFFER_SIZE) || (memcmp(nameOfChunk, IEND, SMALL_BUFFER_SIZE) != 0))
    {
        return ERROR_DATA_INVALID;
    }
    printf("%x %x %x %x\n", nameOfChunk[0], nameOfChunk[1], nameOfChunk[2], nameOfChunk[3]);

    fread(controlSum, 1, SMALL_BUFFER_SIZE, file);
    printf("%x %x %x %x\n", controlSum[0], controlSum[1], controlSum[2], controlSum[3]);
    printf("\n");
    return 0;
}

int readIDAT(FILE *file, unsigned char **arrOfData)
{
    unsigned char const IDAT[4] = { 0x49, 0x44, 0x41, 0x54 };
    unsigned char const IEND[4] = { 0x49, 0x45, 0x4E, 0x44 };
    unsigned char sizeOfChunk[4];
    unsigned char nameOfChunk[4];
    unsigned char controlSum[4];

    while (1)
    {
        size_t bytesRead = fread(sizeOfChunk, 1, SMALL_BUFFER_SIZE, file);
        if (bytesRead != SMALL_BUFFER_SIZE)
        {
            return ERROR_DATA_INVALID;
        }
        printf("%x %x %x %x\n", sizeOfChunk[0], sizeOfChunk[1], sizeOfChunk[2], sizeOfChunk[3]);

        bytesRead = fread(nameOfChunk, 1, SMALL_BUFFER_SIZE, file);
        if (bytesRead != SMALL_BUFFER_SIZE)
        {
            return ERROR_DATA_INVALID;
        }
        printf("%x %x %x %x\n", nameOfChunk[0], nameOfChunk[1], nameOfChunk[2], nameOfChunk[3]);

        int sizeOfChunkInteger = bytesToInt(sizeOfChunk, 0, SMALL_BUFFER_SIZE);
        if ((memcmp(nameOfChunk, IDAT, SMALL_BUFFER_SIZE) != 0))
        {
            if ((memcmp(nameOfChunk, IEND, SMALL_BUFFER_SIZE) != 0))
            {
                if (fseek(file, sizeOfChunkInteger + 4, SEEK_CUR) != 0)
                {
                    return ERROR_DATA_INVALID;
                }
                printf("NOT IDAT\n");
                printf("\n");
                continue;
            }
            else
            {
                if (fseek(file, -8, SEEK_CUR) != 0)
                {
                    return ERROR_DATA_INVALID;
                }
                printf("LAST IDAT WAS\n");
                printf("\n");
                return 0;
            }
        }

        unsigned char inChunk[sizeOfChunkInteger];
        bytesRead = fread(inChunk, 1, sizeOfChunkInteger, file);
        if (bytesRead != sizeOfChunkInteger)
        {
            return ERROR_DATA_INVALID;
        }

        unsigned char *newArray = realloc(*arrOfData, (sizeOfData + sizeOfChunkInteger) * sizeof(unsigned char));
        if (newArray == NULL)
        {
            return ERROR_OUT_OF_MEMORY;
        }
        *arrOfData = newArray;
        for (int i = 0; i < sizeOfChunkInteger; i++)
        {
            (*arrOfData)[sizeOfData + i] = inChunk[i];
        }
        sizeOfData += sizeOfChunkInteger;

        printf("\n");
        fread(controlSum, 1, SMALL_BUFFER_SIZE, file);
        printf("%x %x %x %x\n", controlSum[0], controlSum[1], controlSum[2], controlSum[3]);
        printf("\n");
    }
}

int readPLTE(FILE *file, unsigned char *palette)
{
    unsigned char const PLTE[4] = { 0x50, 0x4C, 0x54, 0x45 };
    unsigned char sizeOfChunk[4];
    unsigned char nameOfChunk[4];
    unsigned char controlSum[4];

    while (1)
    {
        printf("IM HERE");
        size_t bytesRead = fread(sizeOfChunk, 1, SMALL_BUFFER_SIZE, file);
        if ((bytesRead != SMALL_BUFFER_SIZE))
        {
            return ERROR_DATA_INVALID;
        }
        printf("%x %x %x %x\n", sizeOfChunk[0], sizeOfChunk[1], sizeOfChunk[2], sizeOfChunk[3]);

        bytesRead = fread(nameOfChunk, 1, SMALL_BUFFER_SIZE, file);
        if (bytesRead != SMALL_BUFFER_SIZE)
        {
            return ERROR_DATA_INVALID;
        }
        printf("%x %x %x %x\n", nameOfChunk[0], nameOfChunk[1], nameOfChunk[2], nameOfChunk[3]);

        int sizeOfChunkInteger = bytesToInt(sizeOfChunk, 0, SMALL_BUFFER_SIZE);

        if ((memcmp(nameOfChunk, PLTE, SMALL_BUFFER_SIZE) != 0))
        {
            if (fseek(file, sizeOfChunkInteger + 4, SEEK_CUR) != 0)
            {
                return ERROR_DATA_INVALID;
            }
            printf("NOT PLTE\n");
            printf("\n");
            continue;
        }

        if (bytesToInt(sizeOfChunk, 0, SMALL_BUFFER_SIZE) % 3 != 0)
        {
            return ERROR_DATA_INVALID;
        }

        sizeOfPalette = sizeOfChunkInteger;

        bytesRead = fread(palette, 1, sizeOfChunkInteger, file);
        if (bytesRead != sizeOfChunkInteger)
        {
            return ERROR_DATA_INVALID;
        }
        for (int i = 0; i < sizeOfChunkInteger; i++)
        {
            printf("%x ", palette[i]);
        }
        printf("\n");

        fread(controlSum, 1, SMALL_BUFFER_SIZE, file);
        printf("%x %x %x %x\n", controlSum[0], controlSum[1], controlSum[2], controlSum[3]);
        printf("\n");
        return 0;
    }
}

int decompress_deflate_stream(unsigned char *arrOfData, int dataSize, unsigned char *arrOfDecrypt, int decryptSize)
{

#ifdef ZLIB
    uLongf decryptSizeLong = decryptSize;
    int ret = uncompress(arrOfDecrypt, &decryptSizeLong, arrOfData, dataSize);

    if (ret != Z_OK)
        return ERROR_DATA_INVALID;
#endif

#ifdef USE_LIBDEFLATE
    struct libdeflate_decompressor *decompressor = libdeflate_alloc_decompressor();
         if (decompressor == NULL) {
             return ERROR_DATA_INVALID;
         }

         size_t out_nbytes_written;
         int ret = libdeflate_deflate_decompress(decompressor, arrOfData, dataSize, arrOfDecrypt, decryptSize,
         &out_nbytes_written); if (ret != LIBDEFLATE_SUCCESS) {
             libdeflate_free_decompressor(decompressor);
             return ERROR_DATA_INVALID;
         }

         libdeflate_free_decompressor(decompressor);
#endif

#ifdef USE_ISAL
    struct inflate_state state;

    memset(&state, 0, sizeof(state));
    state.next_in = arrOfData;
    state.avail_in = dataSize;
    state.next_out = arrOfDecrypt;
    state.avail_out = decryptSize;

    int ret = isal_inflate(&state);
    if (ret != ISAL_DECOMP_OK) {
        return LIBDEFLATE_BAD_DATA;
    }
#endif
    return 0;
}

int PaethPredictor(int a, int b, int c)
{
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);

    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}

int filter(unsigned char *dataDecrypt, int bpp)
{
    for (int i = 0; i < height; i++)
    {
        int mode = dataDecrypt[i * (width * bpp + 1)];
        for (int j = 1; j <= width; j++)
        {
            int index = i * (width + 1) + j;
            int left = (j - bpp - 1) >= 0 ? dataDecrypt[index - bpp] : 0;
            int above = (i - 1) >= 0 ? dataDecrypt[index - (width + 1)] : 0;
            int upperLeft = (i - 1) >= 0 && (j - bpp - 1) >= 0 ? dataDecrypt[index - (width + 1) - bpp] : 0;

            if (mode == 0)
            {
            }
            else if (mode == 1)
            {
                dataDecrypt[index] = (dataDecrypt[index] + left) % 256;
            }
            else if (mode == 2)
            {
                dataDecrypt[index] = (dataDecrypt[index] + above) % 256;
            }
            else if (mode == 3)
            {
                dataDecrypt[index] = (dataDecrypt[index] + (left + above) / 2) % 256;
            }
            else if (mode == 4)
            {
                dataDecrypt[index] = (dataDecrypt[index] + PaethPredictor(left, above, upperLeft)) % 256;
            }
            else
            {
                return ERROR_DATA_INVALID;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        return ERROR_DATA_INVALID;
    }

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        return ERROR_CANNOT_OPEN_FILE;
    }
    if (isPNG(file) == ERROR_DATA_INVALID)
    {
        fclose(file);
        return ERROR_DATA_INVALID;
    }
    unsigned char *palette = (unsigned char *)malloc(768 * sizeof(unsigned char));
    if (palette == NULL)
    {
        fclose(file);
        return ERROR_OUT_OF_MEMORY;
    }

    readIHDR(file);
    if (colourType == 3)
    {
        int error = readPLTE(file, palette);
        if (error != 0)
        {
            fclose(file);
            return error;
        }
    }

    unsigned char *arrOfData = malloc(10 * sizeof(unsigned char));
    if (arrOfData == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    if (readIDAT(file, &arrOfData) != 0)
    {
        fclose(file);
        return 2;
    }
    if (readIEND(file) != 0)
    {
        fclose(file);
        return 3;
    }

    int deepOnColour = colourType == 2 ? 3 : 1;
    sizeOfDataDecrypt = height * (width * deepOnColour + 1);
    unsigned char *arrOfDecrypt = malloc(sizeOfDataDecrypt * sizeof(unsigned char));
    if (arrOfDecrypt == NULL)
    {
        return ERROR_OUT_OF_MEMORY;
    }

    int error = decompress_deflate_stream(arrOfData, sizeOfData, arrOfDecrypt, sizeOfDataDecrypt);
    if (error != 0)
    {
        fclose(file);
        return error;
    }
    free(arrOfData);

    if (filter(arrOfDecrypt, deepOnColour) != 0)
    {
        return ERROR_DATA_INVALID;
    }

    //    for (int i = 0; i < sizeOfDataDecrypt; i++) {
    //        printf("%x ", arrOfDecrypt[i]);
    //    }
    //    printf("\n");

    FILE *outputFile = fopen(argv[2], "wb");
    if (outputFile == NULL)
    {
        fprintf(stderr, "Error opening output file\n");
        return ERROR_CANNOT_OPEN_FILE;
    }

    int isGrayscale = colourType == 0x00;
    if (colourType == 0x03)
    {
        isGrayscale = 1;
        for (int i = 0; i < sizeOfPalette; i += 3)
        {
            if (palette[i] != palette[i + 1] || palette[i] != palette[i + 2])
            {
                isGrayscale = 0;
                break;
            }
        }
    }

    unsigned char *rowBuffer = isGrayscale ? malloc(width) : malloc(width * 3);
    if (rowBuffer == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        free(arrOfDecrypt);
        fclose(outputFile);
        return ERROR_OUT_OF_MEMORY;
    }

    fprintf(outputFile, isGrayscale ? "P5\n%d %d\n%d\n" : "P6\n%d %d\n%d\n", width, height, (1 << 8) - 1);

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int idx = (i * (width * (deepOnColour) + 1)) + j * deepOnColour + 1;

            if (colourType == 0x00)
            {
                rowBuffer[j] = arrOfDecrypt[idx];
            }
            else if (colourType == 0x03 && isGrayscale)
            {
                unsigned char paletteIndex = arrOfDecrypt[idx];
                rowBuffer[j] = palette[paletteIndex * 3];
            }

            if (!isGrayscale && (colourType == 0x02 || colourType == 0x03))
            {
                unsigned char paletteIndex = arrOfDecrypt[idx];
                for (int k = 0; k < 3; ++k)
                {
                    rowBuffer[j * 3 + k] = palette[paletteIndex * 3 + k];
                }
            }
        }

        size_t bytes_written = fwrite(rowBuffer, 1, isGrayscale ? width : width * 3, outputFile);
        if (bytes_written != (isGrayscale ? width : width * 3))
        {
            fprintf(stderr, "Error writing to output file\n");
            free(rowBuffer);
            free(arrOfDecrypt);
            fclose(outputFile);
            return ERROR_UNKNOWN;
        }
    }

    // Cleanup
    free(rowBuffer);
    free(arrOfDecrypt);
    fclose(outputFile);
    return 0;
}
