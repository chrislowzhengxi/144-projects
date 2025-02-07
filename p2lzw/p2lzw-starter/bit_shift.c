////////////////////////// Encode Implementation //////////////////////////
#include "bit_shift.h"
#include <stdio.h>
#include <stdint.h>



// Global variables to be stored to next iteration 
static uint8_t printed = 0;     
static int current_bits = 0;   

void print_as_bits(FILE *output, uint16_t number, int total_bits) {

    while (total_bits > 0) {
        int space = 8 - current_bits; 
        int bits_to_process;
        if (total_bits >= space) {      
            bits_to_process = space;
        } else {
            bits_to_process = total_bits;
        }

        int shift_amount = bits_to_process; 
        int shifted_value = 1 << shift_amount; 
        int mask = shifted_value - 1;       // (1 << rb) - 1 

        // These steps are from Bo's office hours 
        // Step 1: Mask the first 8 (or n) bits;  Step 3: Fills remaining
        uint8_t added_number = (uint8_t)(number >> (total_bits - bits_to_process)) & mask;
        total_bits -= bits_to_process;  // Reduce remaining bits

        // Step 2: Save the remaining bits to m by bit shift left by 8 (m - 8 bits not printed yet)
        printed = (printed << bits_to_process) | added_number;


        // The next iteration: how many bits to process. 
        current_bits += bits_to_process;

        // fprintf(stderr, "DEBUG ENCODE: number=0x%X, total_bits=%d, added_number=0x%X, printed=0x%X, current_bits=%d\n", 
        //         number, total_bits, added_number, printed, current_bits);


        if (current_bits == 8) {
            putc(printed, output);
            // fprintf(stderr, "DEBUG ENCODE OUTPUT: 0x%02X\n", printed);
            printed = 0;
            current_bits = 0;

        }
    }
}



void print_out_redundant(FILE *output) {
    if (current_bits > 0) {
        // Mask to zero out unused bits
        uint8_t shifting = (printed & ((1 << current_bits) - 1)) << (8 - current_bits);
        putc(shifting, output);

        // Debugging
        // fprintf(stderr, "DEBUG FLUSH: Flushing leftover bits: 0x%02X, current_bits=%d\n", shifting, current_bits);

        // Reset state
        printed = 0;
        current_bits = 0;
    }
}


///// / / //// /


////////////

#include <stdint.h>
#include <stdio.h>

static uint8_t buffer = 0;    
static int buffer_bits = 0;  


int read_bits(FILE *in, int bits) {
    int value = 0;

    while (bits > 0) {

        if (buffer_bits == 0) {
            int next_byte = getc(in);
            if (next_byte == EOF) {
                // ADDED
                if (bits > 0) {
                    // fprintf(stderr, "DEBUG: Insufficient bits to decode. EOF reached.\n");
                    return -1; 
                }
                return -1; // End of input  break; 
            }
            buffer = next_byte;
            buffer_bits = 8; 
        }

        int to_read = (bits < buffer_bits) ? bits : buffer_bits;

        value = (value << to_read) | ((buffer >> (buffer_bits - to_read)) & ((1 << to_read) - 1));

        buffer_bits -= to_read;
        bits -= to_read;
    }


    // // ADDED 
    // if (bits > 0) {
    //     fprintf(stderr, "DEBUG: Not enough bits for a complete code. Remaining bits: %d\n", bits);
    //     return -1; // Signal insufficient bits
    // }

    return value; // Return the decoded value
}


int decode_printed_bits(FILE *input, int *output, int bit_size, int num_values) {
    int value_count = 0;

    while (value_count < num_values) {
        int value = read_bits(input, bit_size); // Read the next `bit_size` bits
        if (value == -1) {
            // fprintf(stderr, "DEBUG DECODE: Reached EOF or insufficient bits.\n");
            return -1; // Signal EOF
        }


        // if (value == 10) {
        //     // fprintf(stderr, "DEBUG: EOF value encountered. Stopping decoding.\n");
        //     // output[value_count++] = value;      // ?? 
        //     break;
        // }

        output[value_count++] = value;
        // fprintf(stderr, "DEBUG DECODE: Decoded value=%d\n", value);
    }

    
    // if (value_count > 0 && output[value_count - 1] == 10) {
    //     putchar('\n'); // Add a newline to the decoded output
    // }

    return value_count; // Return number of decoded values
}
