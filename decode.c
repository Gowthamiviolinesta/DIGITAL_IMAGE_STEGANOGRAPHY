#include <stdio.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#include<string.h>

// Validate decode arguments
Status read_and_validate_decode_args(int argc,char *argv[], decodeInfo *decInfo)
{
    if (argv[2] == NULL)
    {
        printf("Please enter correct arguments\n");
        return e_failure;
    }

    char *dot = strrchr(argv[2], '.');// check extension
    if (dot == NULL || strcasecmp(dot, ".bmp") != 0)// must be .bmp
    {
        printf("Please pass a valid .bmp file\n");
        return e_failure;
    }

    decInfo->stego_image_fname = argv[2];// assign input file

    // Check Output file
    if (argc >=4)
    {
        char *filename = strdup(argv[3]);
        char *dot =strrchr(filename,'.');
        if(dot!=NULL)
        {
            *dot='\0';
        }
        decInfo->output_fname = filename;   // user provided output
    }
    else
    {
        decInfo->output_fname = strdup("decode");   // default output
    }

    return e_success;
}
// Open stego image file
Status open_decode_files(decodeInfo *decInfo)
{
    printf("-----Started decoding-----\n");
   
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");// open in binary read
    if (decInfo->fptr_stego_image == NULL) // check error
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    printf("Open file is success\n");
    return e_success;
}
// Decode magic string
Status decode_magic_string(char *magic, decodeInfo *decInfo)
{
    unsigned char imageBuffer[8];
    int len=strlen(MAGIC_STRING);// length of magic string
    for (int i = 0; i < len; i++)// loop through chars
    {
        //printf("offset pos %ld:\n",ftell(decInfo->fptr_stego_image));
        size_t bytesRead = fread(imageBuffer, 1, 8, decInfo->fptr_stego_image);
        //printf("bytesread:%ld\n",bytesRead);
        if (bytesRead < 8)// error if not read fully
        {
            //printf("offset:%ld\n",ftell(decInfo->fptr_stego_image));
            printf("Error: Could not read 8 bytes for magic string (iteration %d, read %zu)\n", i, bytesRead);
            return e_failure;
        }

        // Decode one char from 8 bytes
        magic[i] = decode_byte_from_lsb((char *)imageBuffer);// decode one char
        //printf("magic:%c\n",magic[i]);
    }

    // Null terminate AFTER the loop
    magic[len] = '\0';

    // Compare after full string decoded
    if (strcmp(magic, MAGIC_STRING) != 0)
    {
        printf("Error: Magic string mismatch (expected: %s, got: %s)\n", MAGIC_STRING, magic);
        return e_failure;
    }
    printf("Magic string matched\n");
    printf("Magic string decoded successfully\n");
    return e_success;
}
// Decode secret file extension size
Status decode_secret_file_extn_size(decodeInfo *decInfo)
{
    char image_buffer[32];
    int size = 0;
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);// read 32 bytes
    size = decode_size_from_lsb(image_buffer);// get size from LSBs
    //printf("size:%d\n",size);
    decInfo->extn_size = size;// store size

    printf("Decoded secret file extension size successfully\n");
    //printf("OFFSET at %ld after decoding file extension size\n", ftell(decInfo->fptr_stego_image))
    return e_success;
}
// Decode secret file extension
Status decode_secret_file_extn(decodeInfo *decInfo)
{
    char extn[decInfo->extn_size + 1];// extension buffer

    // Decode each character of extension from image
    for (int i = 0; i < decInfo->extn_size; i++)// loop each char
    {
        char buffer[8];
        fread(buffer, 8, 1, decInfo->fptr_stego_image);// read 8 bytes
        extn[i] = decode_byte_from_lsb(buffer);// decode one char
    }
    extn[decInfo->extn_size] = '\0';
    // Save extension into structure
    strcpy(decInfo->extn, extn);
    char temp[200];
    if (decInfo->output_fname == NULL || strcmp(decInfo->output_fname, "default") == 0)
    {
        // User didn’t give file → auto-generate with extension
        strcpy(temp,"default");// default name
    }
    else{
        strcpy(temp,decInfo->output_fname);
    }
    strcat(temp,decInfo->extn); //e.g. "decode.txt"
    decInfo->output_fname=strdup(temp); // allocate dynamically
    printf("Decoded secret file extension successfully (%s)\n", decInfo->extn);
    return e_success;
}
// Open output file
Status open_output_file(decodeInfo *decInfo)
{
    decInfo->fptr_secret = fopen(decInfo->output_fname, "wb");
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        return e_failure;
    }
    printf("Opened output file: %s\n", decInfo->output_fname);
    return e_success;
}
// Decode secret file size
Status decode_secret_file_size(decodeInfo *decInfo)
{
    char image_buffer[32];
    int size = 0;
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);// read 32 bytes
    size = decode_size_from_lsb(image_buffer);// decode size
    decInfo->size_secret_file = size;

    //printf("Decoded secret file size: %d\n", decInfo->size_secret_file);
    printf("Decoded secret file size successfully\n");
    return e_success;
}
// Decode secret file data
Status decode_secret_file_data(decodeInfo *decInfo)
{
    char ch;
    char buffer[8];

   // decInfo->fptr_secret=fopen("decode.txt","wb");

    // Step 1 : Loop till secret file size
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        // Step 2 : Read 8 bytes from stego image
        fread(buffer, 8, 1, decInfo->fptr_stego_image);

        // Step 3 : Decode one byte from LSBs
        ch = decode_byte_from_lsb(buffer);

        // Step 4 : Write that character into output file
        fwrite(&ch,1,1,decInfo->fptr_secret);
    }

    fclose(decInfo->fptr_secret);

    printf("Decoded secret file data successfully\n");
    return e_success;
}
// Decode byte from 8 LSBs
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;
    for (int i = 0; i < 8; i++)
    {
        // get LSB
        int bit = image_buffer[i] & 1;

        // shift into position
        data = (data<<1) | bit;
    }
    //printf("data:%c\n",data);
    return data;
}
// Decode integer size from 32 LSBs
int decode_size_from_lsb(const char *image_buffer)
{
    unsigned int size = 0;

    // Loop through 32 bytes → extract 1 LSB each → build 32-bit integer
    for (int i = 0; i < 32; i++)
    {
        // get LSB of each byte
        int bit = image_buffer[i] & 1;

        // Place bit in correct position (MSB first)
        size = (size << 1) | bit;
    }
    //printf("size:%d\n",size);
    return size;
}
// Main decoding function
Status do_decoding(decodeInfo *decInfo)
{
    if (open_decode_files(decInfo) != e_success)// open input
        return e_failure;

    // Skip BMP header before decoding
    if (fseek(decInfo->fptr_stego_image, 54, SEEK_SET) != 0)
    {
        printf("Error: fseek failed\n");
        return e_failure;
    }

    //printf("Current offset after seek = %ld\n", ftell(decInfo->fptr_stego_image));

    char magic[strlen(MAGIC_STRING)+1];
    if (decode_magic_string(magic, decInfo) != e_success) // decode magic
    {
        printf("Error: decode_magic_string failed\n");
        return e_failure;
    }

    if (decode_secret_file_extn_size(decInfo) != e_success)
    {
        printf("Error: decode_secret_file_extn_size failed\n");
        return e_failure;
    }
    if (decode_secret_file_extn(decInfo) != e_success)// decode extn
    {
        printf("Error: decode_secret_file_extn failed\n");
        return e_failure;
    }


    // Step 3: Open output file after extension is known
   if(open_output_file(decInfo)!= e_success) // open output
   {
        printf("Error: open_output_file failed\n");
        return e_failure;
   }

    if (decode_secret_file_size(decInfo) != e_success)// decode size
    {
        printf("Error: decode_secret_file_size failed\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) != e_success)// decode data
    {
        printf("Error: decode_secret_file_data failed\n");
        return e_failure;
    }
    return e_success;   // success case
}
