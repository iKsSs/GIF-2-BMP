//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <math.h>

#define BI_RGB 0

#define SHOW_GIF 1

#ifdef SHOW_GIF
    #define SHOW_HEADER 1
    #define SHOW_RGB_TABLE 0
    #define SHOW_GCE 1
    #define SHOW_IMG_DESC 1
    #define SHOW_DATA_SIZE 1
    #define SHOW_DATA 1
    #define SHOW_END 1
#endif

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
//https://stackoverflow.com/questions/27215610/creating-bmp-file-in-c
//author: Weather Vane
//date: Nov 30 2014
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
