//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#include "gif2bmp.h"

int isBigEndian() {
    int test = 1;
    char *p = (char*)&test;

    return p[0] == 0;
}

void toLittleEndian(const long long int size, WORD *value){
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
	for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {
	
	BYTE padding = 0;
	int padding_count;
	
	BITMAPINFOHEADER bih;

	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = 1000;
	bih.biHeight = 1000;	
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biXPelsPerMeter = 4724;
	bih.biYPelsPerMeter = 4724;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	padding_count = ( 4 - ( (bih.biWidth * bih.biBitCount / 8) % 4) ) % 4;

	bih.biSizeImage = bih.biWidth * bih.biHeight * 3 + bih.biWidth * padding_count;
	
	COLORREF_RGB rgb;
	rgb.cRed = 255;
	rgb.cGreen = 0;
	rgb.cBlue = 0;

	BITMAPFILEHEADER bfh;

	bfh.bfType = 0x424D; 
		toLittleEndian(2, &bfh.bfType);
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + bih.biSize; 
		//toLittleEndian(4, &bfh.bfOffBits);
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih.biSizeImage;
		//toLittleEndian(4, &bfh.bfSize);
	
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, outputFile);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, outputFile);
	
//		printf("%d %d %d\n%d %d %d %d %d\n", sizeof(BITMAPINFOHEADER), sizeof(BITMAPFILEHEADER), sizeof(COLORREF_RGB),
//											sizeof(UINT), sizeof(DWORD), sizeof(LONG), sizeof(WORD), sizeof(BYTE));
	
	int i, j;
	
	for(i = 0; i < bih.biHeight; i++)
	{
		for(j = 0; j < bih.biWidth; j++)
		{
			fwrite(&rgb, sizeof(COLORREF_RGB), 1, outputFile);
		}
		
		//Padding for 4 byte alignment (could be a value other than zero)
		for(j = 0; j < padding_count; j++)
		{
			fwrite(&padding, sizeof(BYTE), 1, outputFile);
		}
	}
	
	return 0;
}
