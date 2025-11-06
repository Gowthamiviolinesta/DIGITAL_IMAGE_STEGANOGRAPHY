#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>

#include "types.h"

// Structure to hold decode related info
typedef struct decodeInfo
{
    char *stego_image_fname;   // stego image file name
    FILE *fptr_stego_image;    // file pointer for stego image

    char *output_fname;        // output file name
    FILE *fptr_secret;         // file pointer for secret file
    FILE *fptr_output;         // extra file pointer if needed
    int size_secret_file;      // size of secret file

    int extn_size;             // size of extension string
    char extn[10];             // store file extension (e.g. ".txt")

} decodeInfo;

Status read_and_validate_decode_args(int argc,char *argv[], decodeInfo *decInfo);

Status do_decoding(decodeInfo *decInfo);

Status open_decode_files(decodeInfo *decInfo);

Status decode_magic_string(char *magic, decodeInfo *decInfo);

char decode_byte_from_lsb(char *image_buffer);

Status decode_secret_file_extn_size(decodeInfo *decInfo);

int decode_size_from_lsb(const char *image_buffer);

Status decode_secret_file_size(decodeInfo *decInfo);

Status decode_secret_file_extn(decodeInfo *decInfo);

Status decode_secret_file_data(decodeInfo *decInfo);

#endif