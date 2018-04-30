# Conversion of file from graphics format GIF to BMP

Library and application in C language for transfer file of graphic format GIF (Graphics Interchange Format) to file of graphics format BMP (Microsoft Windows Bitmap). Files in GIF format are coded by method LZW.

## Author
Jakub Pastuszek - FIT BUT

## Usage
```
gif2bmp [ -h ] [ -i <input_file> ] [ -o <output_file> ] [ -l <log_file> ]
        -i <file>       input *.gif file (default value: stdin)
        -o <file>       output *.bmp file (default value: stdout)
        -l <file>       log file
        -h              help
```

## Implementation

 - Library could read files in graphics format GIF in version **87a** and **89a**
 - GIF file could be **static** or **animated** from **1** to **8** bit **color format**
 - Output BMP file **doesn't contain RLE** compression
 - Translation is is done using Makefile by typping *make* into command line
 - Due to portability there are used predefined types *int8_t*, *u_int32_t* etc., defined in library *sys/types.h*
