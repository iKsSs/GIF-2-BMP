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

#if SHOW_HEADER	
	printf("Head (47494638 3961): %X %X\n",head_pre,head_post);
#endif

	if ( head_pre != 0x47494638 //dec: GIF8
			|| head_post != 0x3961 ) { //dec: 9a
		fprintf(stderr, "Not right head of GIF file\n");
		return -1;
	}
	
	fread(&width, sizeof(WORD), 1, inputFile);
	fread(&height, sizeof(WORD), 1, inputFile);
	
	fread(&GCT, sizeof(BYTE), 1, inputFile);	//Global Color Table specification
	fread(&back_color, sizeof(BYTE), 1, inputFile);
	fread(&pixel_ratio, sizeof(BYTE), 1, inputFile);
// 6:     03 00        3            - logical screen width in pixels
// 8:     05 00        5            - logical screen height in pixels
// A:     F7                        - GCT follows for 256 colors with resolution 3 x 8 bits/primary; the lowest 3 bits represent the bit depth minus 1, the highest true bit means that the GCT is present
// B:     00           0            - background color #0
// C:     00                        - default pixel aspect ratio

#if SHOW_HEADER
	printf("Width\tHeight\tGCT\t\tback_color\tpixel_ratio\n");
	printf("%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\t\t%d (%X)\n",width,width,height,height,GCT,GCT,back_color,back_color,pixel_ratio,pixel_ratio);
#endif

#if SHOW_RGB_TABLE	
	printf("RGB table:\nRed\tGreen\tBlue\n");
#endif

	do {
		//Read color table
		fread(&color.cRed, sizeof(BYTE), 1, inputFile);
		fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
		fread(&color.cBlue, sizeof(BYTE), 1, inputFile);

#if SHOW_RGB_TABLE			
		if ( !(color.cRed == 0x21 && color.cGreen == 0xF9 ) ) {
			printf("%d (%X)\t%d (%X)\t%d (%X)\n",color.cRed,color.cRed,color.cGreen,color.cGreen,color.cBlue,color.cBlue);
		}
#endif
	} while ( !(color.cRed == 0x21 && color.cGreen == 0xF9 ) );

	BYTE GCE, transparency, transparentColor, endOfBlock;
	WORD delay;
	
	GCE = color.cBlue;
	
	fread(&transparency, sizeof(BYTE), 1, inputFile);
	fread(&delay, sizeof(WORD), 1, inputFile);
	fread(&transparentColor, sizeof(BYTE), 1, inputFile);
	fread(&endOfBlock, sizeof(BYTE), 1, inputFile);

#if SHOW_GCE
	printf("------------------\n");
	printf("GCE (21F9): %X%X\n", color.cRed, color.cGreen);
	printf("GCE_dat\ttransp.\tdelay\ttransp_col\tEOB\n");
	printf("%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\n",
				GCE,GCE,transparency,transparency,delay,delay,transparentColor,transparentColor,endOfBlock,endOfBlock);
#endif	
// 30D:   21 F9                    Graphic Control Extension (comment fields precede this in most files)
// 30F:   04           4            - 4 bytes of GCE data follow
// 310:   01                        - there is a transparent background color (bit field; the lowest bit signifies transparency)
// 311:   00 00                     - delay for animation in hundredths of a second: not used
// 313:   10          16            - color #16 is transparent
// 314:   00                        - end of GCE block

	DWORD NWCorner;
	BYTE localColorTable;

	WORD imageWidth, imageHeight;
	
	fread(&imageDesc, sizeof(BYTE), 1, inputFile);

#if SHOW_IMG_DESC	
	printf("------------------\n");
	printf("ImageDesc (2C): %X\n",imageDesc);
#endif

	if ( imageDesc != 0x2c ) {
		fprintf(stderr, "Not right Image Descriptor of GIF file\n");
		return -1;
	}
	
	fread(&NWCorner, sizeof(DWORD), 1, inputFile);
	fread(&imageWidth, sizeof(WORD), 1, inputFile);
	fread(&imageHeight, sizeof(WORD), 1, inputFile);
	fread(&localColorTable, sizeof(BYTE), 1, inputFile);

#if SHOW_IMG_DESC	
	printf("NWcor\timgW\timgH\tlocalColorTable\n");
	printf("%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\n",NWCorner,NWCorner,imageWidth,imageWidth,imageHeight,imageHeight,localColorTable,localColorTable);
#endif
// 315:   2C                       Image Descriptor
// 316:   00 00 00 00 (0,0)         - NW corner position of image in logical screen
// 31A:   03 00 05 00 (3,5)         - image width and height in pixels
// 31E:   00                        - no local color table

	BYTE LZWMinSize, bytesOfEncData;
	
	fread(&LZWMinSize, sizeof(BYTE), 1, inputFile);
// 31F:   08           8           Start of image - LZW minimum code size

#if SHOW_DATA_SIZE
	printf("------------------\n");
	printf("Start of Image - LZW min code size:\n");
	printf("%d (%X)\n",LZWMinSize,LZWMinSize);
#endif	

	int first = 1;

	do {
		fread(&bytesOfEncData, sizeof(BYTE), 1, inputFile);
// 320:   0B          11            - 11 bytes of LZW encoded image data follow

#if SHOW_DATA
	#if SHOW_DATA_SIZE		
			printf("Bytes of enc data\n");
			printf("%d (%X)\n",bytesOfEncData,bytesOfEncData);
	#endif
#else
	#if SHOW_DATA_SIZE		
			if ( first ) {
				printf("Bytes of enc data\n");
				printf("%d (%X)",bytesOfEncData,bytesOfEncData);
				first = 0;
			} else {
				printf(", %d (%X)",bytesOfEncData,bytesOfEncData);
			}
	#endif	
#endif

		for (int i = 0; i < bytesOfEncData; i++) {
			fread(&tmp, sizeof(BYTE), 1, inputFile);
// 321:   00 51 FC 1B 28 70 A0 C1 83 01 01
#if SHOW_DATA
			printf("%2X ",tmp);
#endif
		}
#if SHOW_DATA	
		printf("\n");
#endif
	} while ( bytesOfEncData == 0xFF || bytesOfEncData == 0xFE );

#if !SHOW_DATA	
		printf("\n");
#endif

	int x = -1;

	do {
		x++;
		fread(&endOfImgData, sizeof(BYTE), 1, inputFile);
// 32C:   00                        - end of image data
	} while (endOfImgData != 0x00);
	
	if ( endOfImgData != 0x00 ) {
		fprintf(stderr, "Not right end of image data of GIF file\n");
		return -1;
	}

#if SHOW_END
	printf("------------------\n");
	printf("End (00): %X - %d\n",endOfImgData,x);
#endif

	fread(&term, sizeof(BYTE), 1, inputFile);
// 32D:   3B                       GIF file terminator

#if SHOW_END	
	printf("Term (3B): %X\n",term);
#endif	

	if ( term != 0x3B ) {
		fprintf(stderr, "Not right ending of GIF file\n");
		return -1;
	}

	fseek(inputFile, 0, SEEK_END); // seek to end of file
	gif2bmp->gifSize = ftell(inputFile); // get current file pointer

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
	
	fseek(outputFile, 0, SEEK_END); // seek to end of file
	gif2bmp->bmpSize = ftell(outputFile); // get current file pointer

	return 0;
}
