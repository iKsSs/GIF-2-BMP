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
	
	//Header
	DWORD head_pre;
	WORD head_post;
	WORD canvasWidth, canvasHeight;

	//Logical Screen Descriptor
	BYTE GCT, back_color, pixel_ratio;
	BYTE hasGlobalTable, colorResolution;
	WORD sizeOfGlobalTable;
	
	//Extensions
	BYTE end_of_exts = 0;
	BYTE extension, ext_type;

	//Graphics Control Extension section
	BYTE gce, transparency, transparentColor, endOfBlock;
	WORD delay;

	//Color Table
	COLORREF_RGB color;
	WORD order;

	//{Plain Text, Comment, Application Extension} Extension
	WORD length;
	BYTE c;

	//Image Descriptor
	DWORD NWCorner;
	WORD imageWidth, imageHeight;
	BYTE localColorTable, hasLocalTable;
	WORD sizeOfLocalTable;

	//Image Data
	BYTE LZWMinSize;
	WORD bytesOfEncData;
	BYTE tmp;

	//auxiliary
	int i, j;

//////////////////////////////
// Read GIF file
//////////////////////////////

//********************************************************
//	Header section
//********************************************************

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
	
	fread(&canvasWidth, sizeof(WORD), 1, inputFile);	//canvas width
	fread(&canvasHeight, sizeof(WORD), 1, inputFile);	//canvas height
	
//********************************************************
//	Logical Screen Descriptor section
//********************************************************

	fread(&GCT, sizeof(BYTE), 1, inputFile);	//Global Color Table specification
	fread(&back_color, sizeof(BYTE), 1, inputFile);
	fread(&pixel_ratio, sizeof(BYTE), 1, inputFile);

	hasGlobalTable		= (0x80 & GCT) >> 7;
	colorResolution		= (0x70 & GCT) >> 4;
	sizeOfGlobalTable	= pow (2, (0x07 & GCT) + 1 );

#if SHOW_HEADER
	printf("CanvasW\tCanvasH\tGCT\t\tback_color\tpixel_ratio\n");
	printf("%d (%X)\t%d (%X)\t%X - "BYTE_TO_BINARY_PATTERN"\t%d (%X)\t\t%d (%X)\n",
		canvasWidth,canvasWidth,canvasHeight,canvasHeight,GCT,BYTE_TO_BINARY(GCT),back_color,back_color,pixel_ratio,pixel_ratio);
	if ( hasGlobalTable ) {
		printf("Size of global table: %d\n",sizeOfGlobalTable);
	} else {
		printf("No global table\n");
	}
	printf("Color resolution: %d bits/pixel\n", colorResolution+1);
#endif

//********************************************************
//	Global Color Table section
//********************************************************

#if SHOW_RGB_TABLE	
	printf("RGB table:\nRed\tGreen\tBlue\n");
#endif

	if ( hasGlobalTable ) {
		order = 0;

		for (i = 0; i < sizeOfGlobalTable; ++i) {
			//Read color table
			fread(&color.cRed, sizeof(BYTE), 1, inputFile);
			fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
			fread(&color.cBlue, sizeof(BYTE), 1, inputFile);

#if SHOW_RGB_TABLE
			//printf("%2X\t%d (%X)\t%d (%X)\t%d (%X)\n",order++,color.cRed,color.cRed,color.cGreen,color.cGreen,color.cBlue,color.cBlue);
			printf("%2X\t%X\t%X\t%X\n",order++,color.cRed,color.cGreen,color.cBlue);
#endif
		}
	}

//********************************************************
//	Extensions section
//********************************************************

	do {
		fread(&extension, sizeof(BYTE), 1, inputFile);

		switch (extension) {
case 0x21:

	fread(&ext_type, sizeof(BYTE), 1, inputFile);

	if ( 0xF9 == ext_type ) {
//********************************************************
//	Graphics Control Extension section
//********************************************************
		
		fread(&gce, sizeof(BYTE), 1, inputFile);
		fread(&transparency, sizeof(BYTE), 1, inputFile);
		fread(&delay, sizeof(WORD), 1, inputFile);
		fread(&transparentColor, sizeof(BYTE), 1, inputFile);
		fread(&endOfBlock, sizeof(BYTE), 1, inputFile);

#if SHOW_EXT
		printf("------------------\n");
		printf("GCE (21F9): %X%X\n", extension, ext_type);
		printf("GCE_dat\ttransp.\tdelay\ttransp_col\tEOB\n");
		printf("%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\n",
				gce,gce,transparency,transparency,delay,delay,transparentColor,transparentColor,endOfBlock,endOfBlock);
#endif	

	} else if ( 0x01 == ext_type || 0xFE == ext_type || 0xFF == ext_type ) {
//********************************************************
//	{Plain Text, Comment, Application Extension} Extension section
//********************************************************

#if SHOW_EXT
		printf("------------------\n");
		printf("Plain text/Comment/Application (21{01,FE,FF}): %X%X\n", extension, ext_type);
#endif	

		do {
			fread(&length, sizeof(BYTE), 1, inputFile);

			for (i = 0; i < length; ++i) {
				fread(&c, sizeof(BYTE), 1, inputFile);
#if SHOW_EXT
				printf("%c", c);
#endif	
			}
		} while (length);

#if SHOW_EXT
		printf("\n");
#endif	
	}
break;

case 0x2C:
//********************************************************
//	Image Descriptor section
//********************************************************

#if SHOW_IMG_DESC	
	printf("------------------\n");
	printf("ImageDesc (2C): %X\n",extension);
#endif

	if ( extension != 0x2c ) {
		fprintf(stderr, "Not right Image Descriptor of GIF file\n");
		return -1;
	}
	
	fread(&NWCorner, sizeof(DWORD), 1, inputFile);
	fread(&imageWidth, sizeof(WORD), 1, inputFile);
	fread(&imageHeight, sizeof(WORD), 1, inputFile);
	fread(&localColorTable, sizeof(BYTE), 1, inputFile);

	hasLocalTable 		= (0x80 & localColorTable) >> 7;
	sizeOfLocalTable	= pow (2, (0x07 & localColorTable) + 1 );

#if SHOW_IMG_DESC	
	printf("NWcor\timgW\timgH\tlocalColorTable\tlocalTable\tsizeOfLocalTable\n");
	printf("%d (%X)\t%d (%X)\t%d (%X)\t%X - "BYTE_TO_BINARY_PATTERN"\t%d\t%d\n",
		NWCorner,NWCorner,imageWidth,imageWidth,imageHeight,imageHeight,localColorTable,BYTE_TO_BINARY(localColorTable),hasLocalTable,sizeOfLocalTable);
#endif

//********************************************************
//	Local Color Table section
//********************************************************

	if ( hasLocalTable ) {
		order = 0;

		for (i = 0; i < sizeOfLocalTable; ++i) {
			//Read color table
			fread(&color.cRed, sizeof(BYTE), 1, inputFile);
			fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
			fread(&color.cBlue, sizeof(BYTE), 1, inputFile);

#if SHOW_RGB_TABLE
			//printf("%2X\t%d (%X)\t%d (%X)\t%d (%X)\n",order++,color.cRed,color.cRed,color.cGreen,color.cGreen,color.cBlue,color.cBlue);
			printf("%2X\t%X\t%X\t%X\n",order++,color.cRed,color.cGreen,color.cBlue);
#endif
		}
	}

//********************************************************
//	Image Data section
//********************************************************
	
	fread(&LZWMinSize, sizeof(BYTE), 1, inputFile);

#if SHOW_DATA_SIZE
	printf("------------------\n");
	printf("Start of Image - LZW min code size:\n");
	printf("%d (%X)\n",LZWMinSize,LZWMinSize);
#if !SHOW_DATA
	int first = 1;
#endif
#endif	

	do {
		fread(&bytesOfEncData, sizeof(BYTE), 1, inputFile);

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

		for (i = 0; i < bytesOfEncData; i++) {
			fread(&tmp, sizeof(BYTE), 1, inputFile);
#if SHOW_DATA
			printf("%2X ",tmp);
#endif
		}
#if SHOW_DATA	
		if ( bytesOfEncData ) {
			printf("\n");
		}
#endif
	} while ( 0x00 != bytesOfEncData );

#if SHOW_GIF && !SHOW_DATA	
	printf("\n");
#endif

break;

case 0x3B:
	end_of_exts = 1;
//********************************************************
//	Trailer section
//********************************************************

#if SHOW_END	
	printf("Term (3B): %X\n",extension);
#endif	

break;

default:
	fprintf(stderr, "Wrong ending of GIF file\n");
	return -1;
	
		}
	} while ( !end_of_exts );

//Get file size using standard library
//URL: https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
//Author: Greg Hewgill on 26 Oct 2008
	fseek(inputFile, 0, SEEK_END); // seek to end of file
	gif2bmp->gifSize = ftell(inputFile); // get current file pointer


//////////////////////////////
// Create BMP file
//////////////////////////////
	
	//auxiliary
	BYTE padding = 0;
	int padding_count;

	BITMAPINFOHEADER bih;

	//fill in info header of BMP
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = imageWidth;
	bih.biHeight = imageHeight;	
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

	//fseek(outputFile, 0, SEEK_END); // seek to end of file
	gif2bmp->bmpSize = ftell(outputFile); // get current file pointer

	return 0;
}
