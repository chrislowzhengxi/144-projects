#include "bit_shift.h"
#include <string.h>

#include "bit_shift.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Include your encode and decode functions
#include "bit_shift.h" // Assuming you have print_as_bits and decode_printed_bits

int main() {
    // Step 1: Encode and save the binary file
    FILE *output = fopen("encoded_output.bin", "wb");
    if (!output) {
        perror("Failed to open file for writing");
        return 1;
    }

    // Example: Numbers to encode with varying bit sizes
    // uint16_t numbers[] = {12, 0, 66, 65, 256, 257, 65, 260, 10};
    // int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9}; // Specify the bit sizes for each number

    uint16_t numbers[] = {36, 160, 1030};  
    int bit_sizes[] = {10, 10, 11};
    int num_cases = sizeof(numbers) / sizeof(numbers[0]);

    printf("=== STARTING ENCODE ===\n");
    for (int i = 0; i < num_cases; i++) {
        printf("Encoding value=%d with bit size=%d\n", numbers[i], bit_sizes[i]);
        print_as_bits(output, numbers[i], bit_sizes[i]); // Call your encode function
    }

    // Write out any remaining bits
    print_out_redundant(output); // Call your function to handle leftover bits
    fclose(output);

    printf("Encoding complete. Encoded data saved to encoded_output.bin.\n");

    // Step 2: Decode the binary file
    FILE *input = fopen("encoded_output.bin", "rb");
    if (!input) {
        perror("Failed to open file for reading");
        return 1;
    }

    int decoded_values[num_cases]; // Array to store decoded numbers
    memset(decoded_values, 0, sizeof(decoded_values)); // Initialize array with zeros

    printf("=== STARTING DECODE ===\n");
    for (int i = 0; i < num_cases; i++) {
        int result = decode_printed_bits(input, &decoded_values[i], bit_sizes[i], 1); // Call your decode function
        if (result == -1) {
            printf("Special condition encountered at index=%d\n", i);
            break;
        } else if (result == 0) {
            printf("ERROR: Reached EOF unexpectedly at index=%d\n", i);
            break;
        }

        // Print decoded value
        printf("Decoded value=%d, expected=%d\n", decoded_values[i], numbers[i]);
    }

    fclose(input);

    // Step 3: Validate results
    printf("=== VALIDATION ===\n");
    int all_match = 1;
    for (int i = 0; i < num_cases; i++) {
        if (decoded_values[i] != numbers[i]) {
            all_match = 0;
            printf("ERROR: Mismatch at index %d (original=%d, decoded=%d)\n", i, numbers[i], decoded_values[i]);
        }
    }

    if (all_match) {
        printf("TEST PASSED: All numbers decoded correctly.\n");
    } else {
        printf("TEST FAILED: Decoded values do not match the original.\n");
    }

    return 0;
}






// void test_print_as_bits() {
//     FILE *output = stdout;

//     // Test case: Input sequence of numbers with dynamic bit sizes
//     uint16_t numbers[] = {12, 0, 66, 65, 256, 257, 65, 260, 10};
//     int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9}; // All initially 9 bits

//     // uint16_t numbers[] = {12, 0, 66, 65, 256, 257, 65, 260, 10, 65, 513};   // 36, 16000};
//     // int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 10};  // , 10, 14};

//     // uint16_t numbers[] = {511, 512, 513, 36, 160};
//     // int bit_sizes[] = {9, 10, 10, 10, 10}; 

//     // uint16_t numbers[] = {36, 1023, 512, 513, 160};
//     // int bit_sizes[] = {10, 10, 10, 10, 10};

//     // uint16_t numbers[] = {36, 160, 512};  
//     // int bit_sizes[] = {10, 10, 10};


//     int num_cases = sizeof(numbers) / sizeof(numbers[0]);


//     for (int i = 0; i < num_cases; i++) {
//         print_as_bits(output, numbers[i], bit_sizes[i]);
//     }

//     print_out_redundant(output);
// }

// void test_decode_printed_bits(FILE *input) {

//     // int num_values = 9;
//     // // Decoded output buffer
//     // uint16_t decoded_values[9] = {0};

//     // // 12 0 are 8 bits, the rest are 9 bits
//     // int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9};
//     // for (int i = 0; i < num_values; i++) {
//     //     decode_printed_bits(input, &decoded_values[i], bit_sizes[i], 1); // Decode one value at a time
//     // }

//     // // Extract and print decoded characters (BABAABAAA is after 66, which starts at index 2)
//     // printf("Decoded Output: ");
//     // for (int i = 2; i < num_values; i++) {
//     //     printf("%c", (char)decoded_values[i]);
//     // }

//     int num_values = 9;

//     // CHANGE THIS 
//     int decoded_values[9] = {0};

//     // CHANGE THIS 
//     // int bit_sizes[] = {9, 10, 10, 10, 10}; 
//     int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9};
//     for (int i = 0; i < num_values; i++) {
//         decode_printed_bits(input, &decoded_values[i], bit_sizes[i], 1); // Decode one value at a time
//     }

//     // (BABAABAAA is after 66, which starts at index 2)
//     printf("Decoded Output: ");
//     for (int i = 0; i < num_values; i++) {
//         printf("%c", (char)decoded_values[i]);
//     }

//     printf("\n");
// }



// int main() {
//     int testing_encode = 1; 
//     //  0C00 2110 6010 120C 1014

//     if (!testing_encode) { 
//         FILE *encoded_file = tmpfile(); 
//         if (!encoded_file) {
//             perror("Failed to create temporary file");
//             return 1;
//         }

//         // 
//         uint8_t encoded_data[] = {0xff, 0xc0, 0x10, 0x08, 0x48, 0x50, 0x00};
//         // {511, 512, 513, 36, 160}: 
//         // {36, 160, 512}: {0x09, 0x0a, 0x08, 0x00}; 
//         // {0x0C, 0x00, 0x21, 0x10, 0x60, 0x10, 0x12, 0x0C, 0x10, 0x14};
//         fwrite(encoded_data, 1, sizeof(encoded_data), encoded_file);
//         fflush(encoded_file);
//         rewind(encoded_file);

//         // Decode the bit stream
//         test_decode_printed_bits(encoded_file);

//         fclose(encoded_file); 
//         return 0;
//     } else { 
//         test_print_as_bits();
//         return 0; 
//     }
// }



// ////////////////////////////////////


// // void encode_file(FILE *input, FILE *output) {
// //     int total_bits = 9; // Start with 9 bits encoding (e.g., for 66, 65, etc.)
// //     int max_iterations = 1000000;
// //     int iterations = 0; 
// //     int c;
// //     while ((c = fgetc(input)) != EOF) {
// //         print_as_bits(output, c, total_bits);
// //         // Adjust bit size dynamically here if needed (e.g., when the largest code increases)
   
// //         iterations++;
// //         if (iterations > max_iterations) {
// //             fprintf(stderr, "ERROR: Encoding exceeded maximum iterations. Exiting to prevent infinite loop.\n");
// //             return;
// //         }
   
// //     }




// //     print_out_redundant(output); // Output the remaining bits
// // }

// // void decode_file(FILE *input, FILE *output) {
// //     int decoded_value;
// //     int max_iterations = 1000000; 
// //     int iterations = 0;
// //     int total_bits = 9; // Start with 9 bits decoding
    

// //     while (decode_printed_bits(input, &decoded_value, total_bits, 1)) {
// //         // Write the decoded value as a character to the output file
// //         fputc((char)decoded_value, output);
// //         // Adjust bit size dynamically here if needed (e.g., when the largest code increases)

// //         iterations++;
// //         if (iterations > max_iterations) {
// //             fprintf(stderr, "ERROR: Encoding exceeded maximum iterations. Exiting to prevent infinite loop.\n");
// //             return;
// //         }
// //     }
// // }

// // int main() {
// //     // Open input text file
// //     FILE *input_file = fopen("input.txt", "r");
// //     if (!input_file) {
// //         perror("Failed to open input file");
// //         return 1;
// //     }

// //     // Create temporary file to store encoded binary output
// //     FILE *encoded_file = tmpfile();
// //     if (!encoded_file) {
// //         perror("Failed to create temporary encoded file");
// //         fclose(input_file);
// //         return 1;
// //     }

// //     // Create output file for the decoded text
// //     FILE *decoded_file = fopen("output.txt", "w");
// //     if (!decoded_file) {
// //         perror("Failed to create decoded file");
// //         fclose(input_file);
// //         fclose(encoded_file);
// //         return 1;
// //     }

// //     // Step 1: Encode the input text to the encoded binary file
// //     printf("Encoding...\n");
// //     encode_file(input_file, encoded_file);
// //     fflush(encoded_file);
// //     rewind(encoded_file); // Rewind the file to decode it later

// //     // Step 2: Decode the binary file back to readable text
// //     printf("Decoding...\n");
// //     decode_file(encoded_file, decoded_file);

// //     // Close all files
// //     fclose(input_file);
// //     fclose(encoded_file);
// //     fclose(decoded_file);

// //     printf("Encoding and Decoding completed. Check 'output.txt'.\n");
// //     return 0;
// // }


////////////////////////////////////



// #include <stdio.h>
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include "bit_shift.h" // Include your implementation header

// // Global test variables
// uint16_t test_numbers[] = {12, 0, 66, 65, 256, 257, 65, 260, 10};
// int test_bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9};
// int num_test_cases = sizeof(test_numbers) / sizeof(test_numbers[0]);

// // Function to test encoding logic
// void test_print_as_bits(FILE *output) {
//     for (int i = 0; i < num_test_cases; i++) {
//         fprintf(stderr, "TESTING ENCODE: Encoding number=%u, bit_size=%d\n", test_numbers[i], test_bit_sizes[i]);
//         print_as_bits(output, test_numbers[i], test_bit_sizes[i]);
//     }
//     print_out_redundant(output);
// }

// // Function to test decoding logic
// // void test_decode_printed_bits(FILE *input, int *decoded_numbers) {
// //     for (int i = 0; i < num_test_cases; i++) {
// //         int result = decode_printed_bits(input, &decoded_numbers[i], test_bit_sizes[i], 1);
// //         if (result == -1) {
// //             fprintf(stderr, "TESTING DECODE: Reached EOF or special condition at index=%d\n", i);
// //             break;
// //         }
// //         fprintf(stderr, "TESTING DECODE: Decoded value=%u, expected=%u\n", decoded_numbers[i], test_numbers[i]);
// //     }
// // }

// void test_decode_printed_bits(FILE *input) {
//     int num_values = 9;
//     int decoded_values[9] = {0};
//     int bit_sizes[] = {8, 8, 9, 9, 9, 9, 9, 9, 9};
    
//     for (int i = 0; i < num_values; i++) {
//         int result = decode_printed_bits(input, &decoded_values[i], bit_sizes[i], 1); 
//         if (result == -1) {
//             printf("TESTING DECODE: Special condition at index=%d, value=%d\n", i, decoded_values[i]);
//             break;
//         } else if (result == 0) {
//             printf("TESTING DECODE: Reached EOF or error at index=%d\n", i);
//             break;
//         } else {
//             printf("TESTING DECODE: Decoded value=%d, expected=%d\n", decoded_values[i], decoded_values[i]);
//         }
//     }

//     printf("Decoded Output: ");
//     for (int i = 0; i < num_values; i++) {
//         printf("%c", (char)decoded_values[i]);
//     }
//     printf("\n");
// }


// // Main test function
// int main() {
//     // Step 1: Create a temporary file for encoding
//     FILE *encoded_file = tmpfile();
//     if (!encoded_file) {
//         perror("Failed to create temporary file");
//         return 1;
//     }

//     // Step 2: Encode the test data
//     fprintf(stderr, "=== STARTING ENCODE TEST ===\n");
//     test_print_as_bits(encoded_file);
//     fflush(encoded_file);
//     rewind(encoded_file);

//     // Step 3: Decode the test data
//     int decoded_numbers[num_test_cases];             // int, uint16_t
//     memset(decoded_numbers, 0, sizeof(decoded_numbers));

//     fprintf(stderr, "=== STARTING DECODE TEST ===\n");
//     test_decode_printed_bits(encoded_file);

//     // Step 4: Compare results
//     fprintf(stderr, "=== STARTING VALIDATION ===\n");
//     int all_match = 1;
//     for (int i = 0; i < num_test_cases; i++) {
//         if (test_numbers[i] != decoded_numbers[i]) {
//             all_match = 0;
//             fprintf(stderr, "ERROR: Mismatch at index %d (original: %u, decoded: %u)\n", i, test_numbers[i], decoded_numbers[i]);
//         }
//     }

//     // Print summary
//     if (all_match) {
//         fprintf(stdout, "TEST PASSED: All numbers matched.\n");
//     } else {
//         fprintf(stdout, "TEST FAILED: Some numbers did not match.\n");
//     }

//     fclose(encoded_file);
//     return 0;
// }
