//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#include "gif2bmp.h"

//http://blog.acipo.com/handling-endianness-in-c/
//author: Andrew Ippoliti
//date: 23 Nov 2013

//Check if system is in Big endian
//return	int		1 - is Big endian | 0 - is Little endian
int isBigEndian() {
    int test = 1;
    char *p = (char*)&test;

    return p[0] == 0;
}

//Converts Big endian number to Little endian
//params	size	size of number in bytes
//			value	reference to the number
//return	void
void toLittleEndian(const long long int size, void *value){
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ){
        result[i] = ((char*)value)[size-i-1];
    }
	for( i=0; i<size; i+=1 ){
        ((char*)value)[i] = result[i];
    }
}

// GIF
    // An image (introduced by 0x2C, an ASCII comma ',')
    // An extension block (introduced by 0x21, an ASCII exclamation point '!')
    // The trailer (a single byte of value 0x3B, an ASCII semicolon ';'), which should be the last byte of the file.


//Function to convert GIF file to BMP file
//params	gif2bmp	
//			inputFile	file descriptor to input GIF file
//			outputFile	file descriptor to output BMP file
//return	int			(un)successful conversion
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {
	
	BYTE padding = 0;
	int padding_count;
	
	BYTE tmp;
	
	COLORREF_RGB color;
	
	WORD width, height;
	BYTE GCT, back_color, pixel_ratio; 
	
	DWORD head_pre;
	WORD head_post;
	BYTE imageDesc;
	BYTE endOfImgData;
	BYTE term;
	
	//////////////////////////////
	// Read GIF file
	//////////////////////////////
	
	//47 49 46 38 39 61
	fread(&head_pre, sizeof(DWORD), 1, inputFile);
	fread(&head_post, sizeof(WORD), 1, inputFile);
	
	toLittleEndian(4, &head_pre);
	toLittleEndian(2, &head_post);
	
	printf("Head (47494638 3961):%x %x\n",head_pre,head_post);
	
	if ( head_pre != 0x47494638 //dec: GIF8
			|| head_post != 0x3961 ) { //dec: 9a
		fprintf(stderr, "Not right head of GIF file\n");
		return -1;
	}
	
	fread(&width, sizeof(WORD), 1, inputFile);
	fread(&height, sizeof(WORD), 1, inputFile);
	
	printf("%d (%x) %d (%x)\n",width,width,height,height);
	
	fread(&GCT, sizeof(BYTE), 1, inputFile);	//Global Color Table specification
	fread(&back_color, sizeof(BYTE), 1, inputFile);
	fread(&pixel_ratio, sizeof(BYTE), 1, inputFile);
// 6:     03 00        3            - logical screen width in pixels
// 8:     05 00        5            - logical screen height in pixels
// A:     F7                        - GCT follows for 256 colors with resolution 3 x 8 bits/primary; the lowest 3 bits represent the bit depth minus 1, the highest true bit means that the GCT is present
// B:     00           0            - background color #0
// C:     00                        - default pixel aspect ratio
	
	printf("%d (%x) %d (%x) %d (%x)\n",GCT,GCT,back_color,back_color,pixel_ratio,pixel_ratio);
	
	do {
		//Read color table
		fread(&color.cRed, sizeof(BYTE), 1, inputFile);
		fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
		fread(&color.cBlue, sizeof(BYTE), 1, inputFile);
		
		if ( 0 && !(color.cRed == 0x21 && color.cGreen == 0xF9 ) ) {
			printf("%d (%x) %d (%x) %d (%x)\n",color.cRed,color.cRed,color.cGreen,color.cGreen,color.cBlue,color.cBlue);
		}
	} while ( !(color.cRed == 0x21 && color.cGreen == 0xF9 ) );

	BYTE GCE, transparency, transparentColor, endOfBlock;
	WORD delay;
	
	GCE = color.cBlue;
	
	fread(&transparency, sizeof(BYTE), 1, inputFile);
	fread(&delay, sizeof(WORD), 1, inputFile);
	fread(&transparentColor, sizeof(BYTE), 1, inputFile);
	fread(&endOfBlock, sizeof(BYTE), 1, inputFile);
	
	printf("%d (%x) %d (%x) %d (%x) %d (%x) %d (%x)\n",
				GCE,GCE,transparency,transparency,delay,delay,transparentColor,transparentColor,endOfBlock,endOfBlock);
// 30D:   21 F9                    Graphic Control Extension (comment fields precede this in most files)
// 30F:   04           4            - 4 bytes of GCE data follow
// 310:   01                        - there is a transparent background color (bit field; the lowest bit signifies transparency)
// 311:   00 00                     - delay for animation in hundredths of a second: not used
// 313:   10          16            - color #16 is transparent
// 314:   00                        - end of GCE block

	DWORD NWCorner;
	BYTE localColorTable;

	WORD imageWidth, imageWeight;
	
	fread(&imageDesc, sizeof(BYTE), 1, inputFile);
	
	printf("ImageDesc (2C): %x\n",imageDesc);
	
	if ( imageDesc != 0x2c ) {
		fprintf(stderr, "Not right Image Descriptor of GIF file\n");
		return -1;
	}
	
	fread(&NWCorner, sizeof(DWORD), 1, inputFile);
	fread(&imageWidth, sizeof(WORD), 1, inputFile);
	fread(&imageWeight, sizeof(WORD), 1, inputFile);
	fread(&localColorTable, sizeof(BYTE), 1, inputFile);
	
	printf("%d (%x) %d (%x) %d (%x) %d (%x)\n",NWCorner,NWCorner,imageWidth,imageWidth,imageWeight,imageWeight,localColorTable,localColorTable);
// 315:   2C                       Image Descriptor
// 316:   00 00 00 00 (0,0)         - NW corner position of image in logical screen
// 31A:   03 00 05 00 (3,5)         - image width and height in pixels
// 31E:   00                        - no local color table

	BYTE LZWMinSize, bytesOfEncData;
	
	fread(&LZWMinSize, sizeof(BYTE), 1, inputFile);
	
	printf("%d (%x)",LZWMinSize,LZWMinSize);
	
	do {
		fread(&bytesOfEncData, sizeof(BYTE), 1, inputFile);
		
		printf(", %d (%x)",bytesOfEncData,bytesOfEncData);
// 31F:   08           8           Start of image - LZW minimum code size
// 320:   0B          11            - 11 bytes of LZW encoded image data follow

		for (int i = 0; i < bytesOfEncData; i++) {
			fread(&tmp, sizeof(BYTE), 1, inputFile);
			//printf("%x ",tmp);
		}
		//printf("\n");	
// 321:   00 51 FC 1B 28 70 A0 C1 83 01 01
	} while ( bytesOfEncData == 0xFF || bytesOfEncData == 0xFE );

	printf("\n");
	
	int x = -1;

	do {
		x++;
		fread(&endOfImgData, sizeof(BYTE), 1, inputFile);
	} while (endOfImgData != 0x00);
	
	if ( endOfImgData != 0x00 ) {
		fprintf(stderr, "Not right end of image data of GIF file\n");
		return -1;
	}
	printf("End (00): %x - %d\n",endOfImgData,x);
// 32C:   00                        - end of image data

	fread(&term, sizeof(BYTE), 1, inputFile);
// 32D:   3B                       GIF file terminator
	
	printf("Term (3B): %x\n",term);
	
	if ( term != 0x3B ) {
		fprintf(stderr, "Not right ending of GIF file\n");
		return -1;
	}
	
	return 1;
	//////////////////////////////
	// Create BMP file
	//////////////////////////////
	
	BITMAPINFOHEADER bih;

	//fill in info header of BMP
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height;	
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biXPelsPerMeter = 4724;		//DPI x width
	bih.biYPelsPerMeter = 4724;		//DPI x height
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;

	//compute padding according to width and bit count
	padding_count = ( 4 - ( (bih.biWidth * bih.biBitCount / 8) % 4) ) % 4;

	bih.biSizeImage = bih.biWidth * bih.biHeight * 3 + bih.biWidth * padding_count;
	
	COLORREF_RGB rgb;
	rgb.cRed = 255;
	rgb.cGreen = 0;
	rgb.cBlue = 0;

	BITMAPFILEHEADER bfh;

	//fill in file header of BMP
	bfh.bfType = 0x424D; 	//dec: BM
		toLittleEndian(2, &bfh.bfType);
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + bih.biSize; 	//size of whole header
		//toLittleEndian(4, &bfh.bfOffBits);
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bih.biSizeImage;	//size of whole file
		//toLittleEndian(4, &bfh.bfSize);
	
	gif2bmp->bmpSize = bfh.bfSize;
	
	//write BMP header to outputFile
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, outputFile);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, outputFile);
	
//		printf("%d %d %d\n%d %d %d %d %d\n", sizeof(BITMAPINFOHEADER), sizeof(BITMAPFILEHEADER), sizeof(COLORREF_RGB),
//											sizeof(UINT), sizeof(DWORD), sizeof(LONG), sizeof(WORD), sizeof(BYTE));
	
	int i, j;
	
	for(i = 0; i < bih.biHeight; i++)
	{
		//Write a pixel to outputFile
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
