//*********************************************************//
//* * *               KKO - projekt č. 3              * * *//
//* * *     Konverze obrazového formátu GIF na BMP    * * *//
//* * *                                               * * *//
//* * *                Jakub Pastuszek                * * *//
//* * *           xpastu00@stud.fit.vutbr.cz          * * *//
//* * *                  april 2018                   * * *//
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

	int ret;
	
    int opt;
    extern char *optarg;

	//parse arguments
    while ((opt = getopt(argc, argv, "hi:o:l:")) != -1) {
        switch (opt) {
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
	if ( NULL == inputFile ) {
		fp_i = stdin;
	}
	else {
		fp_i = fopen(inputFile,"rb+");
		
		if ( NULL == fp_i ) {
			fprintf(stderr, "Error: openning %s file\n", inputFile);
			return 1;
		}
	}

	//open output file
	if ( NULL == outputFile ) {
		fp_o = stdout;
	}
	else {
		fp_o = fopen(outputFile,"wb+");
		
		if ( NULL == fp_o ) {
			fprintf(stderr, "Error: openning %s file\n", outputFile);
			fclose(fp_i);
			return 1;
		} 
	}

	//open log file
	if ( NULL != logFile ) {
		fp_l = fopen(logFile,"w");
		
		if ( NULL == fp_l ) {
			fprintf(stderr, "Error: openning %s file\n", logFile);
		} 
	}

	//convert file
	ret = gif2bmp(&record, fp_i, fp_o);

	if ( NULL != inputFile && fclose(fp_i) ) {
		fprintf(stderr, "Error: closing %s file\n", inputFile);
	}
	
	if ( NULL != outputFile && fclose(fp_o) ) {
		fprintf(stderr, "Error: closing %s file\n", outputFile);
	}

	//remove output file if error during convertion
	if ( -1 == ret && NULL != outputFile ) {
		ret = remove(outputFile);
	}

	if ( NULL != logFile ) {
		fprintf(fp_l, "login = xpastu00\n");
		fprintf(fp_l, "uncodedSize = %ld\n", record.gifSize);	//in bytes
		fprintf(fp_l, "codedSize = %ld\n", record.bmpSize);		//in bytes

		if ( fclose(fp_l) ) {
			fprintf(stderr, "Error: closing %s file\n", logFile);
		}
	}
	
	return 0;
}

//Print help message with usage
void printHelpMessage() {
    printf("Usage: gif2bmp [ -h ] [ -i <input_file> ] [ -o <output_file> ] [ -l <log_file> ]\n" \
           "\t-i <file>\tinput *.gif file (default value: stdin)\n" \
           "\t-o <file>\toutput *.bmp file (default value: stdout)\n" \
		   "\t-l <file>\tlog file\n" \
           "\t-h\t\thelp\n");
}
