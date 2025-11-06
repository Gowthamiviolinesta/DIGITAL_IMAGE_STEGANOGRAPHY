#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of the secret file data and return it
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    //rewind(fptr);
    return size;
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //    Step1 : check argv[2] is having .bmp or not
    // true -> store into structure member Step 2 false -> return e_failure
    char* f_name=argv[2];
    int len=strlen(f_name);
    if(!(len > 4 && strcmp(f_name+len-4,".bmp")==0))
    {
       return e_failure;
    }
    encInfo->src_image_fname = argv[2];
    //    Step2 : check argv[3] is having .txt or not
    // true -> store into structure member Step 3 false -> return e_failure
    f_name = argv[3];
    len = strlen(f_name);

    if (!((len > 4 && strcmp(f_name + len - 4, ".txt") == 0) ||
        (len > 2 && strcmp(f_name + len - 2, ".c") == 0)   ||
        (len > 3 && strcmp(f_name + len - 3, ".sh") == 0)))
    {
        return e_failure;
    }
    encInfo->secret_fname = argv[3];
    //   Step 3 : Check argv[4] != NULL or not
    //  true - > Check argv[4] is having .bmp or not false -> Step 4
    // true -> store into structure member return e_success false -> return e-failure
    if (argv[4] != NULL)
    {
        f_name = argv[4];
        len = strlen(f_name);
        if (!(len > 4 && strcmp(f_name + len - 4, ".bmp") == 0))
        {
            return e_failure;
        }
        encInfo->stego_image_fname = argv[4];
    }
    //  Step 4 : assign "default.bmp" into structure member return e_success
    else
    {
        encInfo->stego_image_fname = "default.bmp";
    }

    return e_success;
}
Status open_files(EncodeInfo *encInfo)
{
    printf("-----Started encoding-----\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    printf("Open file is success\n");
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{
    // Step 1 :image_capacity= get the size from  get_image_size() of Bmp function
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    //  Step 2 : Find the size of secret file data by calling this function get_file_size();
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    printf("Secret file size : %ld%s is fit for the encoding\n",encInfo->size_secret_file + 1, encInfo->src_image_fname);
    //  Step 3 : image_ capacity > 54 + 16 + 32 + 32 + 32 + (sizeofdata * 8)
    long required_capacity = 54 + 16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8);
    //  true - > e_success false -> e_failure
    if (encInfo->image_capacity > required_capacity)
    {
        printf("Check capacity is success\n");
        return e_success;
    }
    else
        return e_failure;
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    // Step1 : rewind fptr_src_image
    rewind(fptr_src_image);
    // Step 2: char imageBuffer[54];
    char imageBuffer[54];
    // Step 3 : Read the 54 bytes from src image store into imageBuffer;
    fread(imageBuffer, 54, 1, fptr_src_image);
    // Step 4 : write the 54 bytes to the dest_image;
    fwrite(imageBuffer, 54, 1, fptr_dest_image);
    // return e_success;
    printf("Copied bmp header successfully\n");
    printf("OFFSET at %ld before encode magic string\n",ftell(fptr_src_image));
    return e_success;
}
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    // char imageBuffer[8];
    char imageBuffer[8];
    // Step1 : Find the size of magicstring
    int size=strlen(magic_string);
    // Step 2 :Generate the loop upto the size
    for(int i=0;i<size;i++)
    {
        // Step 3 : Read 8 bytes from src_image store into the imageBuffer
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);
        // Step 4 : Call the encode_byte_to_lsb(magic_string[i] , imageBuffer);
        encode_byte_to_lsb(magic_string[i],imageBuffer);
        // Step 5 : Write the imagebuffer into the dest_image
        fwrite(imageBuffer, 8, 1, encInfo->fptr_stego_image);
        // Step 6 : Repeat the process upto the size
    }
    printf("Encode magic string successfully\n");
    printf("OFFSET at %ld before encodeing extension size \n",ftell(encInfo->fptr_src_image));
    // return e_success;
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    // Char image_Buffer[32];
    char image_buffer[32];
    // Step 1 : Read a 32 bytes from the src_image store into the image_buffer
    fread(image_buffer,32,1,encInfo->fptr_src_image);
    // Step 2 : Call encode_size_to_lsb(size,image_Buffer);
    encode_size_to_lsb(size, image_buffer);
    // Step 3 : write the image_buffer into the dest_image;
    fwrite(image_buffer,32,1,encInfo->fptr_stego_image);
    printf("Encode secret file extension size successfully\n");
    printf("OFFSET at %ld before encode file extension size \n",ftell(encInfo->fptr_src_image));
    // return e_success;
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    // char imageBuffer[8];
    char imagebuffer[8];
    // Step1 : Find the size of extension(.txt)
    int size=strlen(file_extn);
    // Step 2 :Generate the loop upto the size
    for(int i=0;i<size;i++)
    {
        // Step 3 : Read 8 bytes from src_image store into the imageBuffer
        fread(imagebuffer,8,1,encInfo->fptr_src_image);
        // Step 4 : Call the encode_byte_to_lsb(magic_string[i] , imageBuffer);
        encode_byte_to_lsb(file_extn[i] ,imagebuffer);
        // Step 5 : Write the imagebuffer into the dest_image
        fwrite(imagebuffer,8,1,encInfo->fptr_stego_image);
        // Step 6 : Repeat the process upto the size
    }
    printf("Encoded secret file extension successfully\n");
    printf("OFFSET at %ld before encode file size \n",ftell(encInfo->fptr_src_image));

    // return e_success;
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    // Char image_Buffer[32];
    char imagebuffer[32];//to represent 18 bytes u need 32 bits
    // Step 1 : Read a 32 bytes from the src_image store into the image_buffer
    fread(imagebuffer,32,1,encInfo->fptr_src_image);
    //  Step 2 : Call encode_size_to_lsb(size,image_Buffer);
    encode_size_to_lsb(file_size,imagebuffer);
    //  Step 3 : write the image_buffer into the dest_image;
    fwrite(imagebuffer,32,1,encInfo->fptr_stego_image);

    printf("Encoded secret file size successfully\n");
    printf("OFFSET at %ld before file data \n",ftell(encInfo->fptr_src_image));
    // retuen e_succes;
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;

    char imagebuffer[8];// buffer for 8 bytes of image (1 char = 8 bits)
    // Step 1 : rewind the secret_file_fptr;
    rewind(encInfo->fptr_secret);
    // Step 2 :Generate the loop upto the secret_file_size
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(&ch,1,1,encInfo->fptr_secret);
        // Step 3 : Read 8 bytes from src_image store into the imageBuffer
        fread(imagebuffer,8,1,encInfo->fptr_src_image); // read 8 bytes from source image
        // Step 4 : Call the encode_byte_to_lsb(secret_data[i] , imageBuffer);
        encode_byte_to_lsb(ch,imagebuffer);
        // Step 5 : Write the imagebuffer into the dest_image
        fwrite(imagebuffer,8,1,encInfo->fptr_stego_image);
        // Step 6 : Repeat the process upto the size
    }
    printf("Encoded secret file data successfully\n");
    printf("OFFSET at %ld before encode copy remaining data \n",ftell(encInfo->fptr_src_image));
    // return e_success
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    // Copy the remaining data from src to dest
    while((fread(&ch,1,1,fptr_src))>0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }
    // return e_success
    printf("Copied remaining data successfully\n");
    return e_success;
}

/*Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
}*/

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    // Step 1 : Generate the loop from 0 to  7
    for(int i=0;i<8;i++)
    {
        // Step 2 : Clear the lsb bit of Imagebuffer[i];
        image_buffer[i]=image_buffer[i] & (~(1<<0));
        // Step 3 : Get the bit from the data then replace the bit with Imagebuffer[i] of lsb;
        long int bit=((data>>(7-i))&1);
        image_buffer[i]=image_buffer[i]|bit;
        // Step 4 : Reapeat the process upto 7
    }
    return e_success;
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    // Step 1 : Generate the loop from 0 to 31
    for(int i=0;i<32;i++)
    {
        // Step 2 : Clear the lsb bit of Imagebuffer[i];
        image_buffer[i]=image_buffer[i] & (~(1<<0));
        // Step 3 : Get the bit from the size then replace the bit with Imagebuffer[i] of lsb;
        long int bit=((size>>(31-i))&1);
        image_buffer[i]=image_buffer[i]|bit;
        // Step 4 : Reapeat the process upto 31
    }
}
Status do_encoding(EncodeInfo *encInfo)
{
     // Step 1 : Call the open_files(encInfo) check the return value e_success or not
    //   true - > step 2 false -> return e_failure
     if (open_files(encInfo) != e_success)
        return e_failure;
    // Step 2 : Call the Check_capacity(encinfo) check the return value e_success or not;
    //   true - > step 3 false -> return e_failure
    if (check_capacity(encInfo) != e_success)
        return e_failure;
    // Step 3 : Call copybmpHeader()check the return value e_success or not;
    // true - > step 4 false -> return e_failure;
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;
    // Step 4 : Call the encode_magic_string()check the return value e_success or not;
    //   true - > step 5 false -> return e_failure
    if (encode_magic_string("#*", encInfo) != e_success)
        return e_failure;
    // Step 5 : Extract .txt from the secretfile_name and store into structure varaibale then find the size
    char* dot=strchr(encInfo->secret_fname, '.');
    if (!dot)
        return e_failure;
    strcpy(encInfo->extn_secret_file, dot); // store ".txt"
    int extn_size = strlen(encInfo->extn_secret_file);
    //  Step 6: Call the encode secret_file_extension_size() check the return value e_success or not;
    //   true - > step 7 false -> return e_failure
    if (encode_secret_file_extn_size(extn_size, encInfo) != e_success)
        return e_failure;
    //  Step 7: Call the encode secret_file_extension() check the return value e_success or not;
    //   true - > step 8 false -> return e_failure
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
        return e_failure;
    //  Step 8: Call the encode_secret_file_size() check the return value e_success or not;
    //   true - > step 9 false -> return e_failure
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
        return e_failure;
    //  Step 9: Call the encode_secret_file_data() check the return value e_success or not;
    //   true - > step 10 false -> return e_failure
    if (encode_secret_file_data(encInfo) != e_success)
        return e_failure;
    //  //  Step 10: Call the copy_remaining_data() check the return value e_success or not;
    //   true - > step 11 false -> return e_failure
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
        return e_failure;
    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);


    // Step 11 : e_success;
    return e_success;
}
