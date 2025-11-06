/*
Name:-Gowthami S
Date:-31/08/2025
Description:-LSB image Steganography
*/
#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    // Step 1: Must have at least 2 args (program name + operation)
    if (argc < 2)
    {
        printf("Operation not specified.\n");
        printf("Usage:\n");
        printf("  For encoding: ./a.out -e <src_image> <secret.txt> <optional>\n");
        printf("  For decoding: ./a.out -d <stego_image> <optional.txt>\n");
        return e_failure;
    }

    // Step 2: Check operation type
    OperationType opt = check_operation_type(argv[1]);

    if (opt == e_encode)
    {
        // Step 3: Encoding requires at least 4 arguments
        if (argc < 4)
        {
            printf("Usage:\n");
            printf("  For encoding: ./a.out -e <src_image> <secret.txt> <optional>\n");
            printf("  For decoding: ./a.out -d <stego_image> <optional.txt>\n");
            return e_failure;
        }

        EncodeInfo encInfo;
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("Usage:\n");
            printf("  For encoding: ./a.out -e <src_image> <secret.txt> <optional>\n");
            printf("  For decoding: ./a.out -d <stego_image> <optional.txt>\n");
            printf("Encoding validation failed\n");
            return e_failure;
        }

        printf("Encoding: Validation success\n");

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("Encoding failed\n");
            return e_failure;
        }

        printf("Encoding successful!\n");
    }
    else if (opt == e_decode)
    {
        // Step 3: Decoding requires at least 3 arguments
        if (argc < 3)
        {
            printf("Usage:\n");
            printf("  For encoding: ./a.out -e <src_image> <secret.txt> <optional>\n");
            printf("  For decoding: ./a.out -d <stego_image> <optional.txt>\n");
            return e_failure;
        }

        decodeInfo decInfo;
        if (read_and_validate_decode_args(argc,argv, &decInfo) == e_failure)
        {
            printf("Usage:\n");
            printf("  For encoding: ./a.out -e <src_image> <secret.txt> <optional>\n");
            printf("  For decoding: ./a.out -d <stego_image> <optional.txt>\n");
            return e_failure;
        }

        printf("Decoding: Validation success\n");

        if (do_decoding(&decInfo) == e_failure)
        {
            printf("Decoding failed\n");
            return e_failure;
        }

        printf("Decoding successful!!\n");
    }
    else
    {
        printf("Unsupported operation. Use -e for encoding or -d for decoding\n");
        return e_failure;
    }

    return e_success;
}

// Function to check operation type
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0)
    {
        printf("Selected: Encoding\n");
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0)
    {
        printf("Selected: Decoding\n");
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
