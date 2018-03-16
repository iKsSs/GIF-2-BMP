//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  brezen 2018                  * * *//
//*********************************************************//

#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "gif2bmp.h"

void printHelpMessage();

int main(int argc, char *argv[]) {

	char *inputFile = NULL;
	char *outputFile = NULL;
	char *logFile = NULL;

	FILE *fp_i, *fp_o;
	
	tGIF2BMP record;
	int uncodedSize, codedSize;
	
    int opt;
    extern char *optarg;

    while ((opt = getopt(argc, argv, "hi:o:l:")) != -1)
    {
        switch (opt)
        {
        case 'h':       //display help
            printHelpMessage();
            exit(0);
            break;
        case 'i':       //input file
        	inputFile = optarg;
            break;
        case 'o':       //output file
            outputFile = optarg;
            break;
        case 'l':       //log file
            logFile = optarg;
            break;
        default:        //any other param is wrong
            fprintf(stderr, "Wrong parameter. See -h for help\n");
			exit(1);
        }
    }
	
	if ( inputFile == NULL ) {
		fp_i = stdin;
	}
	else {
		fp_i = fopen(inputFile,"rb+");
		
		if ( fp_i == NULL ) {
			fprintf(stderr, "Soubor %s se nepodarilo otevrit\n", inputFile);
			return 1;
		}
	}

	if ( outputFile == NULL ) {
		fp_o = stdout;
	}
	else {
		fp_o = fopen(outputFile,"wb+");
		
		if ( fp_o == NULL ) {
			fprintf(stderr, "Soubor %s se nepodarilo otevrit\n", outputFile);
			fclose(fp_i);
			return 1;
		} 
	}

	gif2bmp(&record, fp_i, fp_o);
	
	if ( inputFile != NULL && fclose(fp_o) ) {
		perror(NULL);
	}
	
	if ( outputFile != NULL && fclose(fp_i) ) {
		perror(NULL);
	}

	// printf("login = xpastu00\n");
	// printf("uncodedSize = %d\n", uncodedSize);	//v bytech
	// printf("codedSize = %d\n", codedSize);		//v bytech
	
	printf("Success\n");
	
	return 0;
}

void printHelpMessage() {
    printf("Usage: gif2bmp -i <input_file> -o <output_file>\n" \
           "\t-i\tinput *.gif file (default value: STDIN)\n" \
           "\t-o\toutput *.bmp file (default value: STDOUT)\n" \
		   "\t-l\tlog file\n" \
           "\t-h\thelp\n");
}
