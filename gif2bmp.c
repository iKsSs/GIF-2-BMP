//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#include "gif2bmp.h"

COLORREF_RGB white = {0xFF,0xFF,0xFF};
COLORREF_RGB black = {0x00,0x00,0x00};

COLORREF_RGB junk = {0xEE,0x33,0x99};

//URL: http://blog.acipo.com/handling-endianness-in-c/
//Author: Andrew Ippoliti on 23 Nov 2013

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
void toLittleEndian(const long long int size, void *value) {
    int i;
    char result[32];
    for( i=0; i<size; i+=1 ) {
        result[i] = ((char*)value)[size-i-1];
    }
	for( i=0; i<size; i+=1 ) {
        ((char*)value)[i] = result[i];
    }
}

//Write debug messages conditioned by define constants
//URL: https://stackoverflow.com/questions/21758136/write-debug-messages-in-c-to-a-file
//Author: unwind on 13 Feb 2014
inline void printDebug(const int show, const char *fmt, ...) {
#ifdef DEBUG
	if ( show ) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stdout,fmt, args);
		va_end(args);
	}	
#endif
}

//Function to convert GIF file to BMP file
//params	gif2bmp	
//			inputFile	file descriptor to input GIF file
//			outputFile	file descriptor to output BMP file
//return	int			(un)successful conversion
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {

//////////////////////////////
// Variable declarations
//////////////////////////////
	//Header
	DWORD head_pre;
	WORD head_post;
	WORD canvasWidth, canvasHeight;

	//Logical Screen Descriptor
	BYTE GCT, back_color, pixel_ratio;
	BYTE hasGlobalTable, colorResolution, sortOfGlobalTable;
	WORD sizeOfGlobalTable;
	
	//Extensions
	BYTE end_of_exts = 0;
	BYTE extension, ext_type;

	//Graphics Control Extension section
	BYTE size, transparency, transparentColor, endOfBlock, gce_loaded, Bnull;
	WORD delay, Wnull;

	//Color Table
	COLORREF_RGB color;
	COLOR_LIST **globalTable;
	COLORREF_RGB *localTable;
	WORD order;
	WORD ClearCode, EOICode;

	//{Plain Text, Comment, Application Extension} Extension
	BYTE length;
	BYTE c;

	//Image Descriptor
	DWORD NWCorner;
	WORD imageWidth, imageHeight;
	BYTE localColorTable, hasLocalTable;
	WORD sizeOfLocalTable;

	//Image Data
	BYTE LZWMinSize, bytesOfEncData;
	int countOfData;
	long int savedPos;
	WORD *gifData;
	BYTE code, lastCode;
	int dataCounter;
	BYTE dataShift;
	BYTE masked;
	BYTE over = 1;

	//Create Table
	int outCounter, colorSize, insertPos;
	COLORREF_RGB *colorBMP;
	COLORREF_RGB currColor, lastColor;
	COLOR_LIST *item, *newColor;

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

	printDebug(SHOW_HEADER,"Head (47494638 3961): %X %X\n",head_pre,head_post);

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
	sortOfGlobalTable	= (0x08 & GCT) >> 3;
	sizeOfGlobalTable	= pow (2, (0x07 & GCT) + 1 );

	printDebug(SHOW_HEADER,"CanvasW\tCanvasH\tGCT\t\tback_color\tpixel_ratio\n");
	printDebug(SHOW_HEADER,"%d (%X)\t%d (%X)\t%X - "BYTE_TO_BINARY_PATTERN"\t%d (%X)\t\t%d (%X)\n",
		canvasWidth,canvasWidth,canvasHeight,canvasHeight,GCT,BYTE_TO_BINARY(GCT),back_color,back_color,pixel_ratio,pixel_ratio);
	if ( hasGlobalTable ) {
		printDebug(SHOW_HEADER,"Size of global table: %d (%d)\n",sizeOfGlobalTable, sortOfGlobalTable);
	} else {
		printDebug(SHOW_HEADER,"No global table\n");
	}
	printDebug(SHOW_HEADER,"Color resolution: %d bits/pixel\n", colorResolution+1);

	//not 8 bit per color
	if ( 7 != colorResolution ) {
		fprintf(stderr, "Unsupported color resolution\n");
		return -1;
	}

//********************************************************
//	Global Color Table section
//********************************************************
	
	printDebug(SHOW_RGB_TABLE,"RGB table:\nRed\tGreen\tBlue\n");

	if ( hasGlobalTable ) {
		order = 0;

		globalTable = (COLOR_LIST**) malloc(sizeof(COLOR_LIST*) * 2 * sizeOfGlobalTable);

		if ( NULL == globalTable ) {
			fprintf(stderr, "Error when allocation Global Table\n");
			return -1;
		}

		for (i = 0; i < 2 * sizeOfGlobalTable; ++i) {
			newColor = (COLOR_LIST*) malloc(sizeof(COLOR_LIST));
			newColor->color = white;
			newColor->valid = 0;
			newColor->next = NULL;

			globalTable[i] = newColor;
		}

		for (i = 0; i < sizeOfGlobalTable; ++i) {
			//Read color table
			fread(&color.cRed, sizeof(BYTE), 1, inputFile);
			fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
			fread(&color.cBlue, sizeof(BYTE), 1, inputFile);

			globalTable[i]->color = color;
			globalTable[i]->valid = 1;

			printDebug(SHOW_RGB_TABLE,"%2X\t%X\t%X\t%X\n",
				order++,color.cRed,color.cGreen,color.cBlue);
		}

		insertPos = sizeOfGlobalTable;

		ClearCode	= insertPos;
		globalTable[insertPos]->valid = 2;
		globalTable[insertPos++]->color	= white;	//Clear Code

		EOICode		= insertPos;
		globalTable[insertPos]->valid = 2;
		globalTable[insertPos++]->color	= white;	//End Of Information Code
	}

//********************************************************
//	Extensions section
//********************************************************

	gce_loaded = 0;

	do {
		fread(&extension, sizeof(BYTE), 1, inputFile);

		switch (extension) {
case 0x21:

	fread(&ext_type, sizeof(BYTE), 1, inputFile);

	if ( 0xF9 == ext_type ) {
//********************************************************
//	Graphics Control Extension section
//********************************************************
		
		fread(&size, sizeof(BYTE), 1, inputFile);

		printDebug(SHOW_EXT,"------------------\n");
		printDebug(SHOW_EXT,"GCE (21F9): %X%X\n", extension, ext_type);

		if ( gce_loaded ) {
			fread(&Bnull, sizeof(BYTE), 1, inputFile);
			fread(&Wnull, sizeof(WORD), 1, inputFile);
			fread(&Bnull, sizeof(BYTE), 1, inputFile);
			fread(&Bnull, sizeof(BYTE), 1, inputFile);
		} else {
			fread(&transparency, sizeof(BYTE), 1, inputFile);
			fread(&delay, sizeof(WORD), 1, inputFile);
			fread(&transparentColor, sizeof(BYTE), 1, inputFile);
			fread(&endOfBlock, sizeof(BYTE), 1, inputFile);

			printDebug(SHOW_EXT,"size\ttransp.\tdelay\ttransp_col\tEOB\n");
			printDebug(SHOW_EXT,"%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\t%d (%X)\n",
					size,size,transparency,transparency,delay,delay,transparentColor,transparentColor,endOfBlock,endOfBlock);

			gce_loaded = 1;
		}

	} else if ( 0x01 == ext_type || 0xFE == ext_type || 0xFF == ext_type ) {
//********************************************************
//	{Plain Text, Comment, Application Extension} Extension section
//********************************************************

		printDebug(SHOW_EXT,"------------------\n");
		printDebug(SHOW_EXT,"Plain text/Comment/Application (21{01,FE,FF}): %X%X\n", extension, ext_type);

		do {
			fread(&length, sizeof(BYTE), 1, inputFile);

			for (i = 0; i < length; ++i) {
				fread(&c, sizeof(BYTE), 1, inputFile);
				printDebug(SHOW_EXT,"%c", c);
			}
		} while (length);

		printDebug(SHOW_EXT,"\n");
	}
break;

case 0x2C:
//********************************************************
//	Image Descriptor section
//********************************************************

	printDebug(SHOW_IMG_DESC,"------------------\n");
	printDebug(SHOW_IMG_DESC,"ImageDesc (2C): %X\n",extension);

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

	printDebug(SHOW_IMG_DESC,"NWcor\timgW\timgH\tlocalColorTable\tlocalTable\tsizeOfLocalTable\n");
	printDebug(SHOW_IMG_DESC,"%d (%X)\t%d (%X)\t%d (%X)\t%X - "BYTE_TO_BINARY_PATTERN"\t%d\t%d\n",
		NWCorner,NWCorner,imageWidth,imageWidth,imageHeight,imageHeight,localColorTable,BYTE_TO_BINARY(localColorTable),hasLocalTable,sizeOfLocalTable);

//********************************************************
//	Local Color Table section
//********************************************************

	if ( hasLocalTable ) {
		order = 0;
		
		localTable = (COLORREF_RGB*) malloc(sizeof(COLORREF_RGB) * sizeOfLocalTable);

		if ( NULL == localTable ) {
			fprintf(stderr, "Error when allocation Local Table\n");
			return -1;
		}

		for (i = 0; i < sizeOfLocalTable; ++i) {
			localTable[i] = white;
		}

		for (i = 0; i < sizeOfLocalTable; ++i) {
			//Read color table
			fread(&color.cRed, sizeof(BYTE), 1, inputFile);
			fread(&color.cGreen, sizeof(BYTE), 1, inputFile);
			fread(&color.cBlue, sizeof(BYTE), 1, inputFile);

			localTable[i] = color;

			//printDebug(SHOW_RGB_TABLE,"%2X\t%d (%X)\t%d (%X)\t%d (%X)\n",order++,color.cRed,color.cRed,color.cGreen,color.cGreen,color.cBlue,color.cBlue);
			printDebug(SHOW_RGB_TABLE,"%2X\t%X\t%X\t%X\n",order++,color.cRed,color.cGreen,color.cBlue);
		}
	}

//********************************************************
//	Image Data section
//********************************************************
	
	fread(&LZWMinSize, sizeof(BYTE), 1, inputFile);

	printDebug(SHOW_DATA_SIZE,"------------------\n");
	printDebug(SHOW_DATA_SIZE,"Start of Image - LZW min code size:\n");
	printDebug(SHOW_DATA_SIZE,"%d (%X)\n",LZWMinSize,LZWMinSize);

	countOfData = 0;

	savedPos = ftell(inputFile);	//save file pointer position

	do {
		fread(&bytesOfEncData, sizeof(BYTE), 1, inputFile);

		countOfData += bytesOfEncData;

		fseek( inputFile, ftell(inputFile)+bytesOfEncData, SEEK_SET );	//shift file pointer position
	} while ( 0x00 != bytesOfEncData );

	fseek( inputFile, savedPos, SEEK_SET );		//recover file pointer position

	countOfData = (countOfData << 3) / 9;		// 9 bit encoded in 8 bit

	gifData = (WORD*) malloc(sizeof(WORD) * countOfData);

	if ( NULL == gifData ) {
		fprintf(stderr, "Error when allocation GIF Data Table\n");
		return -1;
	}

	for (i = 0; i < countOfData; ++i) {
		gifData[i] = 0;
	}

	printDebug(SHOW_DATA_SIZE,"Total data: %d\n",countOfData);

	dataCounter = 0;

	int first = 1;

	do {
		fread(&bytesOfEncData, sizeof(BYTE), 1, inputFile);
		
		printDebug(SHOW_DATA && SHOW_DATA_SIZE,"Bytes of enc data\n");
		printDebug(SHOW_DATA && SHOW_DATA_SIZE,"%d (%X)\n",bytesOfEncData,bytesOfEncData);
		
		if ( first ) {
			printDebug(!SHOW_DATA && SHOW_DATA_SIZE,"Bytes of enc data\n");
			printDebug(!SHOW_DATA && SHOW_DATA_SIZE,"%d (%X)",bytesOfEncData,bytesOfEncData);
			first = 0;
		} else {
			printDebug(!SHOW_DATA && SHOW_DATA_SIZE,", %d (%X)",bytesOfEncData,bytesOfEncData);
		}

		for (i = 0; i < bytesOfEncData; i++) {
			lastCode = code;

			fread(&code, sizeof(BYTE), 1, inputFile);
			
			if ( over ) {
				over = 0;
				dataShift = 0;
			} else {
				gifData[dataCounter++] = lastCode >> dataShift++;

				if ( dataShift == 8 ) {
					over = 1;
				}

				masked = ((0xFF >> (8-dataShift)) & code) << (8 - dataShift);
					
				printDebug(SHOW_DATA_DETAIL,"%2X(%2X): %2X %2X %d - %2X %3X = %3X\n", code, lastCode,
					(0xFF >> (8-dataShift)), ((0xFF >> (8-dataShift)) & code),
					(8 - dataShift), gifData[(dataCounter-1)], masked << 1,
					gifData[(dataCounter-1)] + (masked << 1));

				gifData[(dataCounter-1)] += (masked << 1);
			}

			printDebug(SHOW_DATA,"%2X ",code);
		}

		if ( bytesOfEncData ) {
			printDebug(SHOW_DATA,"\n");
		}
	} while ( 0x00 != bytesOfEncData );
	
	printDebug(!SHOW_DATA,"\n");

	printDebug(SHOW_DATA_DETAIL,"Data detail: ");
	for (i = 0; i < countOfData; i++) {
		printDebug(SHOW_DATA_DETAIL,"%2X ",gifData[i]);
	}
	printDebug(SHOW_DATA_DETAIL,"\n");

break;

case 0x3B:
	end_of_exts = 1;
//********************************************************
//	Trailer section
//********************************************************
	
	printDebug(SHOW_END,"Term (3B): %X\n",extension);

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

//********************************************************
//	Create Table section
//********************************************************
	dataCounter = 0;
	outCounter = 0;

	insertPos = EOICode + 1;

	colorSize = imageWidth * imageHeight;

	colorBMP = (COLORREF_RGB *) malloc(sizeof(COLORREF_RGB) * colorSize);

	if ( NULL == colorBMP ) {
		fprintf(stderr, "Error when allocation Color BMP Table\n");
		return -1;
	}

	for (i = 0; i < colorSize; ++i) {
		colorBMP[i] = junk;
	}

	for (dataCounter = 0; EOICode != gifData[dataCounter]; ++dataCounter) {

		if ( ClearCode == gifData[dataCounter] ) {
			printDebug(SHOW_TABLE,"Clear Code\n");
			//REINIT TABLE
		} else {
			lastColor = currColor;

			item = globalTable[(gifData[dataCounter])];

			currColor = item->color;

			if ( dataCounter ) {	//first color value is only printed out

				if ( item->valid ) {	//color already in table
				 	do {
						colorBMP[outCounter++] = item->color;
				 		if ( NULL == item->next ) { break; }
				 		item = item->next;
				 	} while ( NULL != item );
				}

				globalTable[insertPos]->color = lastColor;
				globalTable[insertPos]->valid = 3;
				
				while ( NULL != item->next ) {
				 	item = item->next;
				}

				newColor = (COLOR_LIST*) malloc(sizeof(COLOR_LIST));
				if ( item->valid ) {
					newColor->color = currColor;
				} else {
					newColor->color = lastColor;
				}
				newColor->valid = 4;
				newColor->next = NULL;

				globalTable[insertPos]->next = newColor;

				item = globalTable[insertPos];

				if ( !item->valid ) {	//new color to table
				 	do {
						colorBMP[outCounter++] = junk;
						if ( NULL == item->next ) { break; }
				 		item = item->next;
				 	} while ( NULL != item );
				}

				insertPos++;
			}

			printDebug(SHOW_TABLE,".");
		}
	}
	
	printDebug(SHOW_TABLE,"\n");

	//for (i = 0; i < sizeOfGlobalTable; ++i) {
	for (i = 0; i < 20; ++i) {
		j = i + sizeOfGlobalTable;
		item = globalTable[i];
		printDebug(SHOW_TABLE,"%2X: %2X %2X %2X %5s %d\t",
			i, item->color.cRed, item->color.cGreen, 
			item->color.cBlue,
			(NULL == item->next) ? "null" : "X",item->valid);
		item = globalTable[j];
		printDebug(SHOW_TABLE,"%2X: %2X %2X %2X %5s %d",
			j, item->color.cRed, item->color.cGreen,
			item->color.cBlue, (NULL == item->next) ? "null" : "X",
			item->valid);
		
		item = globalTable[j]->next;
		while ( NULL != item ) {
			printDebug(SHOW_TABLE,"\t%2X %2X %2X %5s %d",
				item->color.cRed, item->color.cGreen, item->color.cBlue,
				(NULL == item->next) ? "null" : "X",item->valid);
			item = item->next;
		}

		printDebug(SHOW_TABLE,"\n");
	}

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
	
	COLORREF_RGB *currRGB;

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

	printDebug(0,"%d %d %d\n%d %d %d %d %d\n", sizeof(BITMAPINFOHEADER), sizeof(BITMAPFILEHEADER), sizeof(COLORREF_RGB),
											sizeof(UINT), sizeof(DWORD), sizeof(LONG), sizeof(WORD), sizeof(BYTE));

	printDebug(SHOW_OUT_DATA, "Image: %d x %d -> %d\n", imageWidth, imageHeight, imageWidth * imageHeight);
	
	outCounter = 0;

	for(i = 0; i < bih.biHeight; i++)
	{
		//Write a pixel to outputFile
		for(j = 0; j < bih.biWidth; j++)
		{
			currRGB = &(colorBMP[outCounter++]);

			printDebug(SHOW_OUT_DATA,"\x1b[38;2;%d;%d;%dm""X""\x1b[38;2;255;255;255m",
				currRGB->cRed, currRGB->cGreen, currRGB->cBlue);

		//	printDebug(SHOW_OUT_DATA,"%2X;%2X;%2X ",
		//		currRGB->cRed, currRGB->cGreen, currRGB->cBlue);

			fwrite(&currRGB, sizeof(COLORREF_RGB), 1, outputFile);
		}
		
		printDebug(SHOW_OUT_DATA,"\n");

		//Padding for 4 byte alignment (could be a value other than zero)
		for(j = 0; j < padding_count; j++)
		{
			fwrite(&padding, sizeof(BYTE), 1, outputFile);
		}
	}

	//fseek(outputFile, 0, SEEK_END); // seek to end of file
	gif2bmp->bmpSize = ftell(outputFile); // get current file pointer

//********************************************************
//	Deallocation section
//********************************************************

	if ( hasGlobalTable ) {
		free(globalTable);
	}

	if ( hasLocalTable ) {
		free(localTable);
	}

	free(gifData);
	free(colorBMP);

	return 0;
}
