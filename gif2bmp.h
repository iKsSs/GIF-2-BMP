//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  april 2018                   * * *//
//*********************************************************//

#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <math.h>

//////////////////////////////
// Defintions
//////////////////////////////

#define BI_RGB 0

#define MAX_LZW_SIZE 12
#define MAX_TABLE_SIZE pow(2,MAX_LZW_SIZE)

#define BYTE_SIZE_IN_BITS 8

//#define DEBUG

#define SHOW_TEST 1

#define SHOW_GET_N 0

#define SHOW_HEADER 1
#define SHOW_RGB_TABLE 0
#define SHOW_EXT 1
#define SHOW_IMG_DESC 1
#define SHOW_DATA_SIZE 0
#define SHOW_DATA 0
#define SHOW_DATA_DETAIL 0
#define SHOW_END 1
#define SHOW_TABLE 0

#define SHOW_OUT_DATA 0

//Print binary format in printf
//URL: https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
//Author: William Whyte on 8 Jul 2010
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

//////////////////////////////
// Type definitions
//////////////////////////////

typedef uint16_t UINT;	// 2 B
typedef uint32_t DWORD; // 4 B
typedef int32_t LONG;   // 4 B
typedef uint16_t WORD;  // 2 B
typedef uint8_t BYTE;   // 1 B

//////////////////////////////
// Data structures
//////////////////////////////

//Hack to repair incorrect field sizes
//URL: https://stackoverflow.com/questions/27215610/creating-bmp-file-in-c
//Author: Weather Vane on 30 Nov 2014
#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER {
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER;

#pragma pack(pop)

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER;

//24bit = BGR - 8.8.8.0.0
//32bit = BRG - 9.8.7.5.3 | 8.8.8.8.0
typedef struct COLORREF_RGB
{
    BYTE cBlue;
    BYTE cGreen;
    BYTE cRed;
} COLORREF_RGB;

typedef struct COLOR_LIST
{
    COLORREF_RGB color;
    struct COLOR_LIST *next;
    BYTE valid;
} COLOR_LIST;

typedef struct{
	int64_t bmpSize;
	int64_t gifSize;
} tGIF2BMP;

//////////////////////////////
// Functions
//////////////////////////////

extern void printDebug(const int show, const char *fmt, ...);
void toLittleEndian(const long long int size, void *value);
BYTE reverse_byte_binary(BYTE in);
WORD reverse_word_binary(WORD in);
WORD get_n_bits(BYTE *data, BYTE size, int count);
void change_row_interlaced(COLORREF_RGB *to, COLORREF_RGB *from, WORD rowTo, WORD rowFrom, WORD width);
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

#endif
