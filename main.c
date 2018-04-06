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

	FILE *fp_i, *fp_o, *fp_l;
	
	tGIF2BMP record;
	
    int opt;
    extern char *optarg;

	//parse arguments
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
	
	//open input file
	if ( inputFile == NULL ) {
		fp_i = stdin;
	}
	else {
		fp_i = fopen(inputFile,"rb+");
		
		if ( fp_i == NULL ) {
			fprintf(stderr, "Error: openning %s file\n", inputFile);
			return 1;
		}
	}

	//open output file
	if ( outputFile == NULL ) {
		fp_o = stdout;
	}
	else {
		fp_o = fopen(outputFile,"wb+");
		
		if ( fp_o == NULL ) {
			fprintf(stderr, "Error: openning %s file\n", outputFile);
			fclose(fp_i);
			return 1;
		} 
	}

	//open log file
	if ( logFile != NULL ) {
		fp_l = fopen(logFile,"w");
		
		if ( fp_l == NULL ) {
			fprintf(stderr, "Error: openning %s file\n", logFile);
		} 
	}

	gif2bmp(&record, fp_i, fp_o);

	if ( inputFile != NULL && fclose(fp_o) ) {
		fprintf(stderr, "Error: closing %s file\n", inputFile);
	}
	
	if ( outputFile != NULL && fclose(fp_i) ) {
		fprintf(stderr, "Error: closing %s file\n", outputFile);
	}

	if ( logFile != NULL ) {
		fprintf(fp_l, "login = xpastu00\n");
		fprintf(fp_l, "uncodedSize = %ld\n", record.gifSize);	//v bytech
		fprintf(fp_l, "codedSize = %ld\n", record.bmpSize);		//v bytech

		if ( fclose(fp_l) ) {
			fprintf(stderr, "Error: closing %s file\n", logFile);
		}
	}
	
	return 0;
}

//Print help message with usage
void printHelpMessage() {
    printf("Usage: gif2bmp -i <input_file> -o <output_file>\n" \
           "\t-i <file>\tinput *.gif file (default value: stdin)\n" \
           "\t-o <file>\toutput *.bmp file (default value: stdout)\n" \
		   "\t-l <file>\tlog file\n" \
           "\t-h\t\thelp\n");
}
