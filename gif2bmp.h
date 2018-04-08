//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <math.h>

#define BI_RGB 0

#define DEBUG 1

#define SHOW_HEADER 1
#define SHOW_RGB_TABLE 0
#define SHOW_EXT 1
#define SHOW_IMG_DESC 1
#define SHOW_DATA_SIZE 1
#define SHOW_DATA 0
#define SHOW_DATA_DETAIL 1
#define SHOW_END 1
#define SHOW_TABLE 1

#define SHOW_OUT_DATA 1

extern void printDebug(const int show, const char *fmt, ...);

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

typedef uint16_t UINT;	// 2 B
typedef uint32_t DWORD; // 4 B
typedef int32_t LONG;   // 4 B
typedef uint16_t WORD;  // 2 B
typedef uint8_t BYTE;   // 1 B

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

typedef struct{
	int64_t bmpSize;
	int64_t gifSize;
	//int64_t long gifSize;
} tGIF2BMP;

int isBigEndian();

void toLittleEndian(const long long int size, void *value);

/* gif2bmp – záznam o převodu
inputFile – vstupní soubor (GIF)
outputFile – výstupní soubor (BMP)
návratová hodnota – 0 převod proběhl v pořádku, -1 při převodu došlo k chybě, příp. nepodporuje daný formát GIF */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

#endif
